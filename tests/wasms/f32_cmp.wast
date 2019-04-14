(module
  (func (export "eq") (param $x f32) (param $y f32) (result i32) (f32.eq (get_local $x) (get_local $y)))
  (func (export "ne") (param $x f32) (param $y f32) (result i32) (f32.ne (get_local $x) (get_local $y)))
  (func (export "lt") (param $x f32) (param $y f32) (result i32) (f32.lt (get_local $x) (get_local $y)))
  (func (export "le") (param $x f32) (param $y f32) (result i32) (f32.le (get_local $x) (get_local $y)))
  (func (export "gt") (param $x f32) (param $y f32) (result i32) (f32.gt (get_local $x) (get_local $y)))
  (func (export "ge") (param $x f32) (param $y f32) (result i32) (f32.ge (get_local $x) (get_local $y)))
)

