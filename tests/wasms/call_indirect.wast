(module
  ;; Auxiliary definitions
  (type $proc (func))
  (type $out-i32 (func (result i32)))
  (type $out-i64 (func (result i64)))
  (type $out-f32 (func (result f32)))
  (type $out-f64 (func (result f64)))
  (type $over-i32 (func (param i32) (result i32)))
  (type $over-i64 (func (param i64) (result i64)))
  (type $over-f32 (func (param f32) (result f32)))
  (type $over-f64 (func (param f64) (result f64)))
  (type $f32-i32 (func (param f32 i32) (result i32)))
  (type $i32-i64 (func (param i32 i64) (result i64)))
  (type $f64-f32 (func (param f64 f32) (result f32)))
  (type $i64-f64 (func (param i64 f64) (result f64)))
  (type $over-i32-duplicate (func (param i32) (result i32)))
  (type $over-i64-duplicate (func (param i64) (result i64)))
  (type $over-f32-duplicate (func (param f32) (result f32)))
  (type $over-f64-duplicate (func (param f64) (result f64)))

  (func $const-i32 (type $out-i32) (i32.const 0x132))
  (func $const-i64 (type $out-i64) (i64.const 0x164))
  (func $const-f32 (type $out-f32) (f32.const 0xf32))
  (func $const-f64 (type $out-f64) (f64.const 0xf64))

  (func $id-i32 (type $over-i32) (get_local 0))
  (func $id-i64 (type $over-i64) (get_local 0))
  (func $id-f32 (type $over-f32) (get_local 0))
  (func $id-f64 (type $over-f64) (get_local 0))

  (func $i32-i64 (type $i32-i64) (get_local 1))
  (func $i64-f64 (type $i64-f64) (get_local 1))
  (func $f32-i32 (type $f32-i32) (get_local 1))
  (func $f64-f32 (type $f64-f32) (get_local 1))

  (func $over-i32-duplicate (type $over-i32-duplicate) (get_local 0))
  (func $over-i64-duplicate (type $over-i64-duplicate) (get_local 0))
  (func $over-f32-duplicate (type $over-f32-duplicate) (get_local 0))
  (func $over-f64-duplicate (type $over-f64-duplicate) (get_local 0))

  (table (;0;) 29 29 anyfunc) 
  (elem (i32.const 0)
      $const-i32 $const-i64 $const-f32 $const-f64
      $id-i32 $id-i64 $id-f32 $id-f64
      $f32-i32 $i32-i64 $f64-f32 $i64-f64
      $fac-i64 $fib-i64 $even $odd
      $runaway $mutual-runaway1 $mutual-runaway2
      $over-i32-duplicate $over-i64-duplicate
      $over-f32-duplicate $over-f64-duplicate
      $fac-i32 $fac-f32 $fac-f64
      $fib-i32 $fib-f32 $fib-f64)

  ;; Syntax

  (func
    (call_indirect (i32.const 0))
    (call_indirect (param i64) (i64.const 0) (i32.const 0))
    (call_indirect (param i64) (param) (param f64 i32 i64)
      (i64.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (i32.const 0)
    )
    (call_indirect (result) (i32.const 0))
    (drop (i32.eqz (call_indirect (result i32) (i32.const 0))))
    (drop (i32.eqz (call_indirect (result i32) (result) (i32.const 0))))
    (drop (i32.eqz
      (call_indirect (param i64) (result i32) (i64.const 0) (i32.const 0))
    ))
    (drop (i32.eqz
      (call_indirect
        (param) (param i64) (param) (param f64 i32 i64) (param) (param)
        (result) (result i32) (result) (result)
        (i64.const 0) (f64.const 0) (i32.const 0) (i64.const 0) (i32.const 0)
      )
    ))
    (drop (i64.eqz
      (call_indirect (type $over-i64) (param i64) (result i64)
        (i64.const 0) (i32.const 0)
      )
    ))
  )

  ;; Typing

  (func (export "type-i32") (result i32)
    (call_indirect (type $out-i32) (i32.const 0))
  )
  (func (export "type-i64") (result i64)
    (call_indirect (type $out-i64) (i32.const 1))
  )
  (func (export "type-f32") (result f32)
    (call_indirect (type $out-f32) (i32.const 2))
  )
  (func (export "type-f64") (result f64)
    (call_indirect (type $out-f64) (i32.const 3))
  )

  (func (export "type-index") (result i64)
    (call_indirect (type $over-i64) (i64.const 100) (i32.const 5))
  )

  (func (export "type-first-i32") (result i32)
    (call_indirect (type $over-i32) (i32.const 32) (i32.const 4))
  )
  (func (export "type-first-i64") (result i64)
    (call_indirect (type $over-i64) (i64.const 64) (i32.const 5))
  )
  (func (export "type-first-f32") (result f32)
    (call_indirect (type $over-f32) (f32.const 1.32) (i32.const 6))
  )
  (func (export "type-first-f64") (result f64)
    (call_indirect (type $over-f64) (f64.const 1.64) (i32.const 7))
  )

  (func (export "type-second-i32") (result i32)
    (call_indirect (type $f32-i32) (f32.const 32.1) (i32.const 32) (i32.const 8))
  )
  (func (export "type-second-i64") (result i64)
    (call_indirect (type $i32-i64) (i32.const 32) (i64.const 64) (i32.const 9))
  )
  (func (export "type-second-f32") (result f32)
    (call_indirect (type $f64-f32) (f64.const 64) (f32.const 32) (i32.const 10))
  )
  (func (export "type-second-f64") (result f64)
    (call_indirect (type $i64-f64) (i64.const 64) (f64.const 64.1) (i32.const 11))
  )

  ;; Dispatch

  (func (export "dispatch") (param i32 i64) (result i64)
    (call_indirect (type $over-i64) (get_local 1) (get_local 0))
  )

  (func (export "dispatch-structural-i64") (param i32) (result i64)
    (call_indirect (type $over-i64-duplicate) (i64.const 9) (get_local 0))
  )
  (func (export "dispatch-structural-i32") (param i32) (result i32)
    (call_indirect (type $over-i32-duplicate) (i32.const 9) (get_local 0))
  )
  (func (export "dispatch-structural-f32") (param i32) (result f32)
    (call_indirect (type $over-f32-duplicate) (f32.const 9.0) (get_local 0))
  )
  (func (export "dispatch-structural-f64") (param i32) (result f64)
    (call_indirect (type $over-f64-duplicate) (f64.const 9.0) (get_local 0))
  )

  ;; Recursion

  (func $fac-i64 (export "fac-i64") (type $over-i64)
    (if (result i64) (i64.eqz (get_local 0))
      (then (i64.const 1))
      (else
        (i64.mul
          (get_local 0)
          (call_indirect (type $over-i64)
            (i64.sub (get_local 0) (i64.const 1))
            (i32.const 12)
          )
        )
      )
    )
  )

  (func $fib-i64 (export "fib-i64") (type $over-i64)
    (if (result i64) (i64.le_u (get_local 0) (i64.const 1))
      (then (i64.const 1))
      (else
        (i64.add
          (call_indirect (type $over-i64)
            (i64.sub (get_local 0) (i64.const 2))
            (i32.const 13)
          )
          (call_indirect (type $over-i64)
            (i64.sub (get_local 0) (i64.const 1))
            (i32.const 13)
          )
        )
      )
    )
  )

  (func $fac-i32 (export "fac-i32") (type $over-i32)
    (if (result i32) (i32.eqz (get_local 0))
      (then (i32.const 1))
      (else
        (i32.mul
          (get_local 0)
          (call_indirect (type $over-i32)
            (i32.sub (get_local 0) (i32.const 1))
            (i32.const 23)
          )
        )
      )
    )
  )

  (func $fac-f32 (export "fac-f32") (type $over-f32)
    (if (result f32) (f32.eq (get_local 0) (f32.const 0.0))
      (then (f32.const 1.0))
      (else
        (f32.mul
          (get_local 0)
          (call_indirect (type $over-f32)
            (f32.sub (get_local 0) (f32.const 1.0))
            (i32.const 24)
          )
        )
      )
    )
  )

  (func $fac-f64 (export "fac-f64") (type $over-f64)
    (if (result f64) (f64.eq (get_local 0) (f64.const 0.0))
      (then (f64.const 1.0))
      (else
        (f64.mul
          (get_local 0)
          (call_indirect (type $over-f64)
            (f64.sub (get_local 0) (f64.const 1.0))
            (i32.const 25)
          )
        )
      )
    )
  )

  (func $fib-i32 (export "fib-i32") (type $over-i32)
    (if (result i32) (i32.le_u (get_local 0) (i32.const 1))
      (then (i32.const 1))
      (else
        (i32.add
          (call_indirect (type $over-i32)
            (i32.sub (get_local 0) (i32.const 2))
            (i32.const 26)
          )
          (call_indirect (type $over-i32)
            (i32.sub (get_local 0) (i32.const 1))
            (i32.const 26)
          )
        )
      )
    )
  )

  (func $fib-f32 (export "fib-f32") (type $over-f32)
    (if (result f32) (f32.le (get_local 0) (f32.const 1.0))
      (then (f32.const 1.0))
      (else
        (f32.add
          (call_indirect (type $over-f32)
            (f32.sub (get_local 0) (f32.const 2.0))
            (i32.const 27)
          )
          (call_indirect (type $over-f32)
            (f32.sub (get_local 0) (f32.const 1.0))
            (i32.const 27)
          )
        )
      )
    )
  )

  (func $fib-f64 (export "fib-f64") (type $over-f64)
    (if (result f64) (f64.le (get_local 0) (f64.const 1.0))
      (then (f64.const 1.0))
      (else
        (f64.add
          (call_indirect (type $over-f64)
            (f64.sub (get_local 0) (f64.const 2.0))
            (i32.const 28)
          )
          (call_indirect (type $over-f64)
            (f64.sub (get_local 0) (f64.const 1.0))
            (i32.const 28)
          )
        )
      )
    )
  )

  (func $even (export "even") (param i32) (result i32)
    (if (result i32) (i32.eqz (get_local 0))
      (then (i32.const 44))
      (else
        (call_indirect (type $over-i32)
          (i32.sub (get_local 0) (i32.const 1))
          (i32.const 15)
        )
      )
    )
  )
  (func $odd (export "odd") (param i32) (result i32)
    (if (result i32) (i32.eqz (get_local 0))
      (then (i32.const 99))
      (else
        (call_indirect (type $over-i32)
          (i32.sub (get_local 0) (i32.const 1))
          (i32.const 14)
        )
      )
    )
  )

  ;; Stack exhaustion

  ;; Implementations are required to have every call consume some abstract
  ;; resource towards exhausting some abstract finite limit, such that
  ;; infinitely recursive test cases reliably trap in finite time. This is
  ;; because otherwise applications could come to depend on it on those
  ;; implementations and be incompatible with implementations that don't do
  ;; it (or don't do it under the same circumstances).

  (func $runaway (export "runaway") (call_indirect (type $proc) (i32.const 16)))

  (func $mutual-runaway1 (export "mutual-runaway") (call_indirect (type $proc) (i32.const 18)))
  (func $mutual-runaway2 (call_indirect (type $proc) (i32.const 17)))

  ;; As parameter of control constructs and instructions

  (memory 1)

  (func (export "as-select-first") (result i32)
    (select (call_indirect (type $out-i32) (i32.const 0)) (i32.const 2) (i32.const 3))
  )
  (func (export "as-select-mid") (result i32)
    (select (i32.const 2) (call_indirect (type $out-i32) (i32.const 0)) (i32.const 3))
  )
  (func (export "as-select-last") (result i32)
    (select (i32.const 2) (i32.const 3) (call_indirect (type $out-i32) (i32.const 0)))
  )

  (func (export "as-if-condition") (result i32)
    (if (result i32) (call_indirect (type $out-i32) (i32.const 0)) (then (i32.const 1)) (else (i32.const 2)))
  )

  (func (export "as-br_if-first") (result i64)
    (block (result i64) (br_if 0 (call_indirect (type $out-i64) (i32.const 1)) (i32.const 2)))
  )
  (func (export "as-br_if-last") (result i32)
    (block (result i32) (br_if 0 (i32.const 2) (call_indirect (type $out-i32) (i32.const 0))))
  )

  (func (export "as-br_table-first") (result f32)
    (block (result f32) (call_indirect (type $out-f32) (i32.const 2)) (i32.const 2) (br_table 0 0))
  )
  (func (export "as-br_table-last") (result i32)
    (block (result i32) (i32.const 2) (call_indirect (type $out-i32) (i32.const 0)) (br_table 0 0))
  )

  (func (export "as-store-first")
    (call_indirect (type $out-i32) (i32.const 0)) (i32.const 1) (i32.store)
  )
  (func (export "as-store-last")
    (i32.const 10) (call_indirect (type $out-f64) (i32.const 3)) (f64.store)
  )

  (func (export "as-memory.grow-value") (result i32)
    (memory.grow (call_indirect (type $out-i32) (i32.const 0)))
  )
  (func (export "as-return-value") (result i32)
    (call_indirect (type $over-i32) (i32.const 1) (i32.const 4)) (return)
  )
  (func (export "as-drop-operand")
    (call_indirect (type $over-i64) (i64.const 1) (i32.const 5)) (drop)
  )
  (func (export "as-br-value") (result f32)
    (block (result f32) (br 0 (call_indirect (type $over-f32) (f32.const 1) (i32.const 6))))
  )
  (func (export "as-set_local-value") (result f64)
    (local f64) (set_local 0 (call_indirect (type $over-f64) (f64.const 1) (i32.const 7))) (get_local 0)
  )
  (func (export "as-tee_local-value") (result f64)
    (local f64) (tee_local 0 (call_indirect (type $over-f64) (f64.const 1) (i32.const 7)))
  )
  (global $a (mut f64) (f64.const 10.0))
  (func (export "as-set_global-value") (result f64)
    (set_global $a (call_indirect (type $over-f64) (f64.const 1.0) (i32.const 7)))
    (get_global $a)
  )

  (func (export "as-load-operand") (result i32)
    (i32.load (call_indirect (type $out-i32) (i32.const 0)))
  )

  (func (export "as-unary-operand") (result f32)
    (block (result f32)
      (f32.sqrt
        (call_indirect (type $over-f32) (f32.const 0x0p+0) (i32.const 6))
      )
    )
  )

  (func (export "as-binary-left") (result i32)
    (block (result i32)
      (i32.add
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
        (i32.const 10)
      )
    )
  )
  (func (export "as-binary-right") (result i32)
    (block (result i32)
      (i32.sub
        (i32.const 10)
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
      )
    )
  )

  (func (export "as-test-operand") (result i32)
    (block (result i32)
      (i32.eqz
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
      )
    )
  )

  (func (export "as-compare-left") (result i32)
    (block (result i32)
      (i32.le_u
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
        (i32.const 10)
      )
    )
  )
  (func (export "as-compare-right") (result i32)
    (block (result i32)
      (i32.ne
        (i32.const 10)
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
      )
    )
  )

  (func (export "as-convert-operand") (result i64)
    (block (result i64)
      (i64.extend_s/i32
        (call_indirect (type $over-i32) (i32.const 1) (i32.const 4))
      )
    )
  )

)

