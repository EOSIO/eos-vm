(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\05\04\01\02\01\01" ;; memory
  )
  "invalid limits"
)

(module (memory 65536))
