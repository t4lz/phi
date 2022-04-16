use std::future::Future;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::task::{Context, Poll, RawWaker, RawWakerVTable, Waker};
use wasmtime::*;
use wasmtime_wasi::sync::WasiCtxBuilder;
use std::time::Instant;


fn dummy_waker() -> Waker {
    return unsafe { Waker::from_raw(clone(5 as *const _)) };

    unsafe fn clone(ptr: *const ()) -> RawWaker {
        assert_eq!(ptr as usize, 5);
        const VTABLE: RawWakerVTable = RawWakerVTable::new(clone, wake, wake_by_ref, drop);
        RawWaker::new(ptr, &VTABLE)
    }

    unsafe fn wake(ptr: *const ()) {
        assert_eq!(ptr as usize, 5);
    }

    unsafe fn wake_by_ref(ptr: *const ()) {
        assert_eq!(ptr as usize, 5);
    }

    unsafe fn drop(ptr: *const ()) {
        assert_eq!(ptr as usize, 5);
    }
}

fn build_engine() -> Arc<Engine> {
    let mut config = Config::new();
    config.async_support(true);
    config.epoch_interruption(true);
    Arc::new(Engine::new(&config).unwrap())
}


fn get_linker_with_import(engine: &Engine) -> Linker<()> {
    let mut linker = Linker::new(engine);
    let engine = engine.clone();

    linker
        .func_new(
            "phi_client",
            "injected_import",
            FuncType::new(None, None),
            move |_caller, _params, _results| {
                engine.increment_epoch();
                Ok(())
            },
        )
        .unwrap();

    linker
}


/// Run a test with the given wasm, giving an initial deadline of
/// `initial` ticks in the future, and either configuring the wasm to
/// yield and set a deadline `delta` ticks in the future if `delta` is
/// `Some(..)` or trapping if `delta` is `None`.
///
/// Returns `Some(yields)` if function completed normally, giving the
/// number of yields that occured, or `None` if a trap occurred.
fn run_and_count_yields_or_trap<F: Fn(Arc<Engine>)>(
    wasm: &Vec<u8>,
    initial: u64,
    delta: Option<u64>,
    setup_func: F,
) -> anyhow::Result<()> {
    let engine = build_engine();
    let mut linker = Linker::new(&engine);
    wasmtime_wasi::add_to_linker(&mut linker, |s| s)?;

    // Create a WASI context and put it in a Store; all instances in the store
    // share this context. `WasiCtxBuilder` provides a number of ways to
    // configure what the target program will have access to.
    let wasi = WasiCtxBuilder::new()
        .inherit_stdio()
        .inherit_args()?
        .build();
    let mut store = Store::new(&engine, wasi);
    let module = unsafe {Module::deserialize(&engine, engine.precompile_module(&wasm)?)?};

    linker.module(&mut store, "", &module)?;



    store.set_epoch_deadline(initial);
    match delta {
        Some(delta) => {
            store.epoch_deadline_async_yield_and_update(delta);
        }
        None => {
            store.epoch_deadline_trap();
        }
    }

    let engine_clone = engine.clone();
    setup_func(engine_clone);

    let mut start = Instant::now();
    let mut end = Instant::now();
    {
        let mut future = Box::pin(async {
            let instance = linker.instantiate_async(&mut store, &module).await.unwrap();
            let f = instance.get_func(&mut store, "_start").unwrap();
            start = Instant::now();
            f.call_async(&mut store, &[], &mut []).await
        });
        let mut yields = 0;
        loop {
            match future
                .as_mut()
                .poll(&mut Context::from_waker(&dummy_waker()))
            {
                Poll::Ready(Ok(..)) => {
                    end = Instant::now();
                    break;
                }
                Poll::Pending => {
                    yields += 1;
                }
                Poll::Ready(Err(e)) => match e.downcast::<wasmtime::Trap>() {
                    Ok(_) => {
                        end = Instant::now();
                        break;
                    }
                    e => {
                        e.unwrap();
                    }
                },
            }
        }
    }
    println!("Elapsed: {:.2?}", end.duration_since(start));

    Ok(())
}

fn run_normal(
    wasm: &str,
    initial: u64,
    phi: bool,
) -> anyhow::Result<()> {
    // Define the WASI functions globally on the `Config`.
    let engine = Engine::default();
    // let mut linker;
    // if phi{
    //     linker = get_linker_with_import(&engine);
    // } else {
    //     linker = Linker::new(&engine);
    // }
    let mut linker = Linker::new(&engine);
    wasmtime_wasi::add_to_linker(&mut linker, |s| s)?;

    // Create a WASI context and put it in a Store; all instances in the store
    // share this context. `WasiCtxBuilder` provides a number of ways to
    // configure what the target program will have access to.
    let wasi = WasiCtxBuilder::new()
        .inherit_stdio()
        .inherit_args()?
        .build();
    let mut store = Store::new(&engine, wasi);

    // Instantiate our module with the imports we've created, and run it.
    let module = Module::from_file(&engine, wasm)?;
    linker.module(&mut store, "", &module)?;
    let func = linker
        .get_default(&mut store, "")?
        .typed::<(), (), _>(&store)?;

    for i in 1..100 {
        let now = Instant::now();
        func.call(&mut store, ())?;
        println!("Elapsed: {:.2?}", now.elapsed());
    }

    Ok(())
}

// #[test]
// fn epoch_yield_at_func_entry() {
//     // Should yield at start of call to func $subfunc.
//     assert_eq!(
//         Some(1),
//         run_and_count_yields_or_trap(
//             "
//             (module
//                 (import \"\" \"bump_epoch\" (func $bump))
//                 (func (export \"run\")
//                     call $bump  ;; bump epoch
//                     call $subfunc) ;; call func; will notice new epoch and yield
//                 (func $subfunc))
//             ",
//             1,
//             Some(1),
//             |_| {},
//         )
//     );
// }
// 
// #[test]
// fn epoch_yield_at_loop_header() {
//     // Should yield at top of loop, once per five iters.
//     assert_eq!(
//         Some(2),
//         run_and_count_yields_or_trap(
//             "
//             (module
//                 (import \"\" \"bump_epoch\" (func $bump))
//                 (func (export \"run\")
//                     (local $i i32)
//                     (local.set $i (i32.const 10))
//                     (loop $l
//                         call $bump
//                         (br_if $l (local.tee $i (i32.sub (local.get $i) (i32.const 1)))))))
//             ",
//             0,
//             Some(5),
//             |_| {},
//         )
//     );
// }
// 
// #[test]
// fn epoch_yield_immediate() {
//     // We should see one yield immediately when the initial deadline
//     // is zero.
//     assert_eq!(
//         Some(1),
//         run_and_count_yields_or_trap(
//             "
//             (module
//                 (import \"\" \"bump_epoch\" (func $bump))
//                 (func (export \"run\")))
//             ",
//             0,
//             Some(1),
//             |_| {},
//         )
//     );
// }
// 
// #[test]
// fn epoch_yield_only_once() {
//     // We should yield from the subfunction, and then when we return
//     // to the outer function and hit another loop header, we should
//     // not yield again (the double-check block will reload the correct
//     // epoch).
//     assert_eq!(
//         Some(1),
//         run_and_count_yields_or_trap(
//             "
//             (module
//                 (import \"\" \"bump_epoch\" (func $bump))
//                 (func (export \"run\")
//                   (local $i i32)
//                   (call $subfunc)
//                   (local.set $i (i32.const 0))
//                   (loop $l
//                     (br_if $l (i32.eq (i32.const 10)
//                                       (local.tee $i (i32.add (i32.const 1) (local.get $i)))))))
//                 (func $subfunc
//                   (call $bump)))
//             ",
//             1,
//             Some(1),
//             |_| {},
//         )
//     );
// }
// 
// fn epoch_interrupt_infinite_loop() {
//     assert_eq!(
//         None,
//         run_and_count_yields_or_trap(
//             "
//             (module
//                 (import \"\" \"bump_epoch\" (func $bump))
//                 (func (export \"run\")
//                   (loop $l
//                     (br $l))))
//             ",
//             1,
//             None,
//             |engine| {
//                 std::thread::spawn(move || {
//                     std::thread::sleep(std::time::Duration::from_millis(50));
//                     engine.increment_epoch();
//                 });
//             },
//         )
//     );
// }


fn main() -> anyhow::Result<()>{
    let args: Vec<String> = std::env::args().collect();
    if args.len() == 1 {
        panic!("Please pass filename of wasm module to run.");
    }
    let wasm = std::fs::read(&args[1])?;

    for i in 1..100 {
        run_and_count_yields_or_trap(
            &wasm,
            0,
            Some(1),
            |engine| {
                std::thread::spawn(move || {
                    std::thread::sleep(std::time::Duration::from_millis(1));
                    engine.increment_epoch();
                });
            },
            // |_| {},
        )?;
    }
    println!("normal:");

    run_normal(&args[1], 0, false)
}
