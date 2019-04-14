(module
  (func (export "abs") (param $x f32) (result f32) (f32.abs (get_local $x)))
  (func (export "neg") (param $x f32) (result f32) (f32.neg (get_local $x)))
  (func (export "copysign") (param $x f32) (param $y f32) (result f32) (f32.copysign (get_local $x) (get_local $y)))
)

