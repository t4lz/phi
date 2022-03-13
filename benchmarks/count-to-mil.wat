;; Author: Tal Zwick

(module
    (func (export "main")
        (local $i i32)
        (local.set $i (i32.const 0))

        (loop 

            ;; Increment loop counter.
            (local.set $i (i32.add (local.get $i) (i32.const 1)))

            ;;(br_if 1 (i32.eq (local.get $i) (i32.const -1)))
            ;; impossible condition:
            (br_if 1 (i32.eq (local.get $i) (i32.const 1000000)))
            (br 0)
        )
    )
)
