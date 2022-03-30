(module
  (type (;0;) (func (param i32)))
  (import "wasi_snapshot_preview1" "proc_exit" (func (;0;) (type 0)))
  (func (export "_start")
    i32.const 42
    call 0
    unreachable)
)
