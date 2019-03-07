(module
  (type (;0;) (func (param i32)(result i32)))
  (type (;1;) (func))
  (type (;2;) (func (param i32)))

  (import "env" "assert" (func (;0;) (type 2)))
  (import "env" "pi" (func (;1;) (type 0)))

  (func $8u_good1 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_u offset=0 
  )

  (func $8u_good2 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_u align=1 
  )

  (func $8u_good3 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_u offset=1 align=1 
  )

  (func $8u_good4 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_u offset=2 align=1
  )

  (func $8u_good5 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_u offset=25 align=1
  )

  (func $8s_good1 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_s offset=0 
  )

  (func $8s_good2 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_s align=1 
  )

  (func $8s_good3 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_s offset=1 align=1 
  )

  (func $8s_good4 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_s offset=2 align=1
  )

  (func $8s_good5 (type 0)(param i32)(result i32)
        get_local 0
        i32.load8_s offset=25 align=1
  )

  (func $16u_good1 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_u offset=0 
  )

  (func $16u_good2 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_u align=1 
  )

  (func $16u_good3 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_u offset=1 align=1 
  )

  (func $16u_good4 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_u offset=2 align=1
  )

  (func $16u_good5 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_u offset=25 align=1
  )

  (func $16s_good1 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_s offset=0 
  )

  (func $16s_good2 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_s align=1 
  )

  (func $16s_good3 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_s offset=1 align=1 
  )

  (func $16s_good4 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_s offset=2 align=1
  )

  (func $16s_good5 (type 0)(param i32)(result i32)
        get_local 0
        i32.load16_s offset=25 align=1
  )

  (func $run_good8u (type 1)
      i32.const 0
      call $8u_good1
      call 1
      i32.const 97
      i32.eq
      call 0 

      i32.const 0
      call $8u_good2
      call 1
      i32.const 97
      i32.eq
      call 0

      i32.const 0
      call $8u_good3
      call 1
      i32.const 98
      i32.eq
      call 0

      i32.const 0
      call $8u_good4
      call 1
      i32.const 99
      i32.eq
      call 0

      i32.const 0
      call $8u_good5
      call 1
      i32.const 122
      i32.eq
      call 0)

  (func $main (type 1)
      call $run_good8u
  )

  (export "main" (func $main))
  (memory (;0;) 1)
  (data (i32.const 0) "abcdefghijklmnopqrstuvwyz")
)
