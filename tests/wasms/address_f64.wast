(module
  (memory 1)
  (data (i32.const 0) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\f4\7f\01\00\00\00\00\00\fc\7f")

  (func (export "64_good1") (param $i i32) (result f64)
    (f64.load offset=0 (get_local $i))                     ;; 0.0 '\00\00\00\00\00\00\00\00'
  )
  (func (export "64_good2") (param $i i32) (result f64)
    (f64.load align=1 (get_local $i))                      ;; 0.0 '\00\00\00\00\00\00\00\00'
  )
  (func (export "64_good3") (param $i i32) (result f64)
    (f64.load offset=1 align=1 (get_local $i))             ;; 0.0 '\00\00\00\00\00\00\00\00'
  )
  (func (export "64_good4") (param $i i32) (result f64)
    (f64.load offset=2 align=2 (get_local $i))             ;; 0.0 '\00\00\00\00\00\00\00\00'
  )
  (func (export "64_good5") (param $i i32) (result f64)
    (f64.load offset=18 align=8 (get_local $i))            ;; nan:0xc000000000001 '\01\00\00\00\00\00\fc\7f'
  )
  (func (export "64_bad") (param $i i32)
    (drop (f64.load offset=4294967295 (get_local $i)))
  )
)




