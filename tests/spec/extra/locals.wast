(module
  (func $fail (result i64)
    (local i64)
    (local.get 0)
  )
  (func (export "local-zero-init") (result i64)
    (drop (i64.const 374625143947436)) ;; jit specific -- the top level function is called with RAX=0, so we need a layer of indirection to trigger a failure.
    (call $fail)
  )
)

(assert_return (invoke "local-zero-init") (i64.const 0))
