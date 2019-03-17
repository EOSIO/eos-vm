(module
  (memory 1)
  (data (i32.const 0) "abcdefghijklmnopqrstuvwxyz")

  (func (export "8u_good1") (param $i i32) (result i32)
    (i32.load8_u offset=0 (get_local $i))                   ;; 97 'a'
  )
  (func (export "8u_good2") (param $i i32) (result i32)
    (i32.load8_u align=1 (get_local $i))                    ;; 97 'a'
  )
  (func (export "8u_good3") (param $i i32) (result i32)
    (i32.load8_u offset=1 align=1 (get_local $i))           ;; 98 'b'
  )
  (func (export "8u_good4") (param $i i32) (result i32)
    (i32.load8_u offset=2 align=1 (get_local $i))           ;; 99 'c'
  )
  (func (export "8u_good5") (param $i i32) (result i32)
    (i32.load8_u offset=25 align=1 (get_local $i))          ;; 122 'z'
  )

  (func (export "8s_good1") (param $i i32) (result i32)
    (i32.load8_s offset=0 (get_local $i))                   ;; 97 'a'
  )
  (func (export "8s_good2") (param $i i32) (result i32)
    (i32.load8_s align=1 (get_local $i))                    ;; 97 'a'
  )
  (func (export "8s_good3") (param $i i32) (result i32)
    (i32.load8_s offset=1 align=1 (get_local $i))           ;; 98 'b'
  )
  (func (export "8s_good4") (param $i i32) (result i32)
    (i32.load8_s offset=2 align=1 (get_local $i))           ;; 99 'c'
  )
  (func (export "8s_good5") (param $i i32) (result i32)
    (i32.load8_s offset=25 align=1 (get_local $i))          ;; 122 'z'
  )

  (func (export "16u_good1") (param $i i32) (result i32)
    (i32.load16_u offset=0 (get_local $i))                  ;; 25185 'ab'
  )
  (func (export "16u_good2") (param $i i32) (result i32)
    (i32.load16_u align=1 (get_local $i))                   ;; 25185 'ab'
  )
  (func (export "16u_good3") (param $i i32) (result i32)
    (i32.load16_u offset=1 align=1 (get_local $i))          ;; 25442 'bc'
  )
  (func (export "16u_good4") (param $i i32) (result i32)
    (i32.load16_u offset=2 align=2 (get_local $i))          ;; 25699 'cd'
  )
  (func (export "16u_good5") (param $i i32) (result i32)
    (i32.load16_u offset=25 align=2 (get_local $i))         ;; 122 'z\0'
  )

  (func (export "16s_good1") (param $i i32) (result i32)
    (i32.load16_s offset=0 (get_local $i))                  ;; 25185 'ab'
  )
  (func (export "16s_good2") (param $i i32) (result i32)
    (i32.load16_s align=1 (get_local $i))                   ;; 25185 'ab'
  )
  (func (export "16s_good3") (param $i i32) (result i32)
    (i32.load16_s offset=1 align=1 (get_local $i))          ;; 25442 'bc'
  )
  (func (export "16s_good4") (param $i i32) (result i32)
    (i32.load16_s offset=2 align=2 (get_local $i))          ;; 25699 'cd'
  )
  (func (export "16s_good5") (param $i i32) (result i32)
    (i32.load16_s offset=25 align=2 (get_local $i))         ;; 122 'z\0'
  )

  (func (export "32_good1") (param $i i32) (result i32)
    (i32.load offset=0 (get_local $i))                      ;; 1684234849 'abcd'
  )
  (func (export "32_good2") (param $i i32) (result i32)
    (i32.load align=1 (get_local $i))                       ;; 1684234849 'abcd'
  )
  (func (export "32_good3") (param $i i32) (result i32)
    (i32.load offset=1 align=1 (get_local $i))              ;; 1701077858 'bcde'
  )
  (func (export "32_good4") (param $i i32) (result i32)
    (i32.load offset=2 align=2 (get_local $i))              ;; 1717920867 'cdef'
  )
  (func (export "32_good5") (param $i i32) (result i32)
    (i32.load offset=25 align=4 (get_local $i))             ;; 122 'z\0\0\0'
  )

  (func (export "8u_bad") (param $i i32)
    (drop (i32.load8_u offset=4294967295 (get_local $i)))
  )
  (func (export "8s_bad") (param $i i32)
    (drop (i32.load8_s offset=4294967295 (get_local $i)))
  )
  (func (export "16u_bad") (param $i i32)
    (drop (i32.load16_u offset=4294967295 (get_local $i)))
  )
  (func (export "16s_bad") (param $i i32)
    (drop (i32.load16_s offset=4294967295 (get_local $i)))
  )
  (func (export "32_bad") (param $i i32)
    (drop (i32.load offset=4294967295 (get_local $i)))
  )
)




