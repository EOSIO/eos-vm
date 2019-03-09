(module
  (memory 1)
  (data (i32.const 0) "abcdefghijklmnopqrstuvwxyz")
  (data (i32.const 26) "\00\00\00\00\00\00\a0\7f\01\00\d0\7f")

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
  (func (export "8u_good641") (param $i i32) (result i64)
    (i64.load8_u offset=0 (get_local $i))                   ;; 97 'a'
  )
  (func (export "8u_good642") (param $i i32) (result i64)
    (i64.load8_u align=1 (get_local $i))                    ;; 97 'a'
  )
  (func (export "8u_good643") (param $i i32) (result i64)
    (i64.load8_u offset=1 align=1 (get_local $i))           ;; 98 'b'
  )
  (func (export "8u_good644") (param $i i32) (result i64)
    (i64.load8_u offset=2 align=1 (get_local $i))           ;; 99 'c'
  )
  (func (export "8u_good645") (param $i i32) (result i64)
    (i64.load8_u offset=25 align=1 (get_local $i))          ;; 122 'z'
  )

  (func (export "8s_good641") (param $i i32) (result i64)
    (i64.load8_s offset=0 (get_local $i))                   ;; 97 'a'
  )
  (func (export "8s_good642") (param $i i32) (result i64)
    (i64.load8_s align=1 (get_local $i))                    ;; 97 'a'
  )
  (func (export "8s_good643") (param $i i32) (result i64)
    (i64.load8_s offset=1 align=1 (get_local $i))           ;; 98 'b'
  )
  (func (export "8s_good644") (param $i i32) (result i64)
    (i64.load8_s offset=2 align=1 (get_local $i))           ;; 99 'c'
  )
  (func (export "8s_good645") (param $i i32) (result i64)
    (i64.load8_s offset=25 align=1 (get_local $i))          ;; 122 'z'
  )

  (func (export "16u_good641") (param $i i32) (result i64)
    (i64.load16_u offset=0 (get_local $i))                 ;; 25185 'ab'
  )
  (func (export "16u_good642") (param $i i32) (result i64)
    (i64.load16_u align=1 (get_local $i))                  ;; 25185 'ab'
  )
  (func (export "16u_good643") (param $i i32) (result i64)
    (i64.load16_u offset=1 align=1 (get_local $i))         ;; 25442 'bc'
  )
  (func (export "16u_good644") (param $i i32) (result i64)
    (i64.load16_u offset=2 align=2 (get_local $i))         ;; 25699 'cd'
  )
  (func (export "16u_good645") (param $i i32) (result i64)
    (i64.load16_u offset=25 align=2 (get_local $i))        ;; 122 'z\0'
  )

  (func (export "16s_good641") (param $i i32) (result i64)
    (i64.load16_s offset=0 (get_local $i))                 ;; 25185 'ab'
  )
  (func (export "16s_good642") (param $i i32) (result i64)
    (i64.load16_s align=1 (get_local $i))                  ;; 25185 'ab'
  )
  (func (export "16s_good643") (param $i i32) (result i64)
    (i64.load16_s offset=1 align=1 (get_local $i))         ;; 25442 'bc'
  )
  (func (export "16s_good644") (param $i i32) (result i64)
    (i64.load16_s offset=2 align=2 (get_local $i))         ;; 25699 'cd'
  )
  (func (export "16s_good645") (param $i i32) (result i64)
    (i64.load16_s offset=25 align=2 (get_local $i))        ;; 122 'z\0'
  )

  (func (export "32u_good641") (param $i i32) (result i64)
    (i64.load32_u offset=0 (get_local $i))                 ;; 1684234849 'abcd'
  )
  (func (export "32u_good642") (param $i i32) (result i64)
    (i64.load32_u align=1 (get_local $i))                  ;; 1684234849 'abcd'
  )
  (func (export "32u_good643") (param $i i32) (result i64)
    (i64.load32_u offset=1 align=1 (get_local $i))         ;; 1701077858 'bcde'
  )
  (func (export "32u_good644") (param $i i32) (result i64)
    (i64.load32_u offset=2 align=2 (get_local $i))         ;; 1717920867 'cdef'
  )
  (func (export "32u_good645") (param $i i32) (result i64)
    (i64.load32_u offset=25 align=4 (get_local $i))        ;; 122 'z\0\0\0'
  )

  (func (export "32s_good641") (param $i i32) (result i64)
    (i64.load32_s offset=0 (get_local $i))                 ;; 1684234849 'abcd'
  )
  (func (export "32s_good642") (param $i i32) (result i64)
    (i64.load32_s align=1 (get_local $i))                  ;; 1684234849 'abcd'
  )
  (func (export "32s_good643") (param $i i32) (result i64)
    (i64.load32_s offset=1 align=1 (get_local $i))         ;; 1701077858 'bcde'
  )
  (func (export "32s_good644") (param $i i32) (result i64)
    (i64.load32_s offset=2 align=2 (get_local $i))         ;; 1717920867 'cdef'
  )
  (func (export "32s_good645") (param $i i32) (result i64)
    (i64.load32_s offset=25 align=4 (get_local $i))        ;; 122 'z\0\0\0'
  )

  (func (export "64_good641") (param $i i32) (result i64)
    (i64.load offset=0 (get_local $i))                     ;; 0x6867666564636261 'abcdefgh'
  )
  (func (export "64_good642") (param $i i32) (result i64)
    (i64.load align=1 (get_local $i))                      ;; 0x6867666564636261 'abcdefgh'
  )
  (func (export "64_good643") (param $i i32) (result i64)
    (i64.load offset=1 align=1 (get_local $i))             ;; 0x6968676665646362 'bcdefghi'
  )
  (func (export "64_good644") (param $i i32) (result i64)
    (i64.load offset=2 align=2 (get_local $i))             ;; 0x6a69686766656463 'cdefghij'
  )
  (func (export "64_good645") (param $i i32) (result i64)
    (i64.load offset=25 align=8 (get_local $i))            ;; 122 'z\0\0\0\0\0\0\0'
  )

  (func (export "8u_bad64") (param $i i32)
    (drop (i64.load8_u offset=4294967295 (get_local $i)))
  )
  (func (export "8s_bad64") (param $i i32)
    (drop (i64.load8_s offset=4294967295 (get_local $i)))
  )
  (func (export "16u_bad64") (param $i i32)
    (drop (i64.load16_u offset=4294967295 (get_local $i)))
  )
  (func (export "16s_bad64") (param $i i32)
    (drop (i64.load16_s offset=4294967295 (get_local $i)))
  )
  (func (export "32u_bad64") (param $i i32)
    (drop (i64.load32_u offset=4294967295 (get_local $i)))
  )
  (func (export "32s_bad64") (param $i i32)
    (drop (i64.load32_s offset=4294967295 (get_local $i)))
  )
  (func (export "64_bad64") (param $i i32)
    (drop (i64.load offset=4294967295 (get_local $i)))
  )

  (func (export "f32_good1") (param $i i32) (result f32)
    (f32.load offset=26 (get_local $i))                   ;; 0.0 '\00\00\00\00'
  )
  (func (export "f32_good2") (param $i i32) (result f32)
    (f32.load offset=26 align=1 (get_local $i))                    ;; 0.0 '\00\00\00\00'
  )
  (func (export "f32_good3") (param $i i32) (result f32)
    (f32.load offset=27 align=1 (get_local $i))           ;; 0.0 '\00\00\00\00'
  )
  (func (export "f32_good4") (param $i i32) (result f32)
    (f32.load offset=28 align=2 (get_local $i))           ;; 0.0 '\00\00\00\00'
  )
  (func (export "f32_good5") (param $i i32) (result f32)
    (f32.load offset=34 align=4 (get_local $i))           ;; nan:0x500001 '\01\00\d0\7f'
  )
  (func (export "f32_bad") (param $i i32)
    (drop (f32.load offset=4294967295 (get_local $i)))
  )
)
