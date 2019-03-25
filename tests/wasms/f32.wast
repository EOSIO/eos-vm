(module
  (func (export "add") (param $x f32) (param $y f32) (result f32) (f32.add (get_local $x) (get_local $y)))
  (func (export "sub") (param $x f32) (param $y f32) (result f32) (f32.sub (get_local $x) (get_local $y)))
  (func (export "mul") (param $x f32) (param $y f32) (result f32) (f32.mul (get_local $x) (get_local $y)))
  (func (export "div") (param $x f32) (param $y f32) (result f32) (f32.div (get_local $x) (get_local $y)))
  (func (export "sqrt") (param $x f32) (result f32) (f32.sqrt (get_local $x)))
  (func (export "min") (param $x f32) (param $y f32) (result f32) (f32.min (get_local $x) (get_local $y)))
  (func (export "max") (param $x f32) (param $y f32) (result f32) (f32.max (get_local $x) (get_local $y)))
  (func (export "ceil") (param $x f32) (result f32) (f32.ceil (get_local $x)))
  (func (export "floor") (param $x f32) (result f32) (f32.floor (get_local $x)))
  (func (export "trunc") (param $x f32) (result f32) (f32.trunc (get_local $x)))
  (func (export "nearest") (param $x f32) (result f32) (f32.nearest (get_local $x)))
)

