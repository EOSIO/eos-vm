(module binary "msa\00")
(module binary "msa\00\01\00\00\00")
(module binary "msa\00\00\00\00\01")
(module binary "asm\01\00\00\00\00")
(module binary "wasm\01\00\00\00")
(module binary "\7fasm\01\00\00\00")
(module binary "\80asm\01\00\00\00")
(module binary "\82asm\01\00\00\00")
(module binary "\ffasm\01\00\00\00")
(amodule binary "\00\00\00\01msa\00")
(amodule binary "a\00ms\00\01\00\00")
(amodule binary "sm\00a\00\00\01\00")
(amodule binary "\00ASM\01\00\00\00")
(amodule binary "\00\81\a2\94\01\00\00\00")
(amodule binary "\ef\bb\bf\00asm\01\00\00\00")
(amodule binary "\00asm")
(amodule binary "\00asm\01")
(amodule binary "\00asm\01\00\00")
(amodule binary "\00asm\00\00\00\00")
(amodule binary "\00asm\0d\00\00\00")
(amodule binary "\00asm\0e\00\00\00")
(amodule binary "\00asm\00\01\00\00")
(amodule binary "\00asm\00\00\01\00")
(amodule binary "\00asm\00\00\00\01")
(module binary "\00asm\01\00\00\00\05\04\01\00\82\00")
(module binary "\00asm\01\00\00\00\05\07\01\00\82\80\80\80\00")
(module binary "\00asm\01\00\00\00\06\07\01\7f\00\41\80\00\0b)
(module binary "\00asm\01\00\00\00\06\07\01\7f\00\41\ff\7f\0b")
(module binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\80\80\80\80\00\0b")
(module binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\ff\ff\ff\ff\7f\0b")
(module binary "\00asm\01\00\00\00\06\07\01\7e\00\42\80\00\0b")
(module binary "\00asm\01\00\00\00\06\07\01\7e\00\42\ff\7f\0b")
(module binary "\00asm\01\00\00\00\06\0f\01\7e\00\42\80\80\80\80\80\80\80\80\80\00\0b")
(module binary "\00asm\01\00\00\00\06\0f\01\7e\00\42\ff\ff\ff\ff\ff\ff\ff\ff\ff\7f\0b")
(module binary "\00asm\01\00\00\00\05\03\01\00\00\0b\07\01\80\00\41\00\0b\00)
(module binary "\00asm\01\00\00\00\04\04\01\70\00\00\09\07\01\80\00\41\00\0b\00)
(amodule binary "\00asm\01\00\00\00\05\08\01\00\82\80\80\80\80\00")
(amodule binary "\00asm\01\00\00\00\06\0b\01\7f\00\41\80\80\80\80\80\00\0b")
(amodule binary "\00asm\01\00\00\00\06\0b\01\7f\00\41\ff\ff\ff\ff\ff\7f\0b")
(amodule binary "\00asm\01\00\00\00\06\10\01\7e\00\42\80\80\80\80\80\80\80\80\80\80\00\0b")
(amodule binary "\00asm\01\00\00\00\06\10\01\7e\00\42\ff\ff\ff\ff\ff\ff\ff\ff\ff\ff\7f\0b")
(amodule binary "\00asm\01\00\00\00\05\07\01\00\82\80\80\80\70)
(amodule binary "\00asm\01\00\00\00\05\07\01\00\82\80\80\80\40)
(amodule binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\80\80\80\80\70\0b")
(amodule binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\ff\ff\ff\ff\0f\0b")
(amodule binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\80\80\80\80\1f\0b")
(amodule binary "\00asm\01\00\00\00\06\0a\01\7f\00\41\ff\ff\ff\ff\4f\0b")
(amodule binary  "\00asm\01\00\00\00\06\0f\01\7e\00\42\80\80\80\80\80\80\80\80\80\7e\0b")
(amodule binary  "\00asm\01\00\00\00\06\0f\01\7e\00\42\ff\ff\ff\ff\ff\ff\ff\ff\ff\01\0b")
(amodule binary  "\00asm\01\00\00\00\06\0f\01\7e\00\42\80\80\80\80\80\80\80\80\80\02\0b")
(amodule binary "\00asm\01\00\00\00\06\0f\01\7e\00\42\ff\ff\ff\ff\ff\ff\ff\ff\ff\41\0b")
(amodule binary "\00asm\01\00\00\00\01\04\01\60\00\00\03\02\01\00\04\04\01\70\00\00\0a\09\01\07\00\41\00\11\00\01\0b")
(amodule binary "\00asm\01\00\00\00\01\04\01\60\00\00\03\02\01\00\04\04\01\70\00\00\0a\0a\01\07\00\41\00\11\00\80\00\0b")
(amodule binary "\00asm\01\00\00\00\01\04\01\60\00\00\03\02\01\00\04\04\01\70\00\00\0a\0b\01\08\00\41\00\11\00\80\80\00\0b")
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"      ;; Type section
    "\03\02\01\00"            ;; Function section
    "\04\04\01\70\00\00"      ;; Table section
    "\0a\0c\01"               ;; Code section

    ;; function 0
    "\09\00"
    "\41\00"                   ;; i32.const 0
    "\11\00"                   ;; call_indirect (type 0)
    "\80\80\80\00"             ;; call_indirect reserved byte
    "\0b"                      ;; end
  )
  "zero flag expected"
)

(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"      ;; Type section
    "\03\02\01\00"            ;; Function section
    "\04\04\01\70\00\00"      ;; Table section
    "\0a\0d\01"               ;; Code section

    ;; function 0
    "\0a\00"
    "\41\00"                   ;; i32.const 0
    "\11\00"                   ;; call_indirect (type 0)
    "\80\80\80\80\00"          ;; call_indirect reserved byte
    "\0b"                      ;; end
  )
  "zero flag expected"
)

;; memory.grow reserved byte equal to zero.
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\09\01"                ;; Code section

    ;; function 0
    "\07\00"
    "\41\00"                   ;; i32.const 0
    "\40"                      ;; memory.grow
    "\01"                      ;; memory.grow reserved byte is not equal to zero!
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

;; memory.grow reserved byte should not be a "long" LEB128 zero.
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0a\01"                ;; Code section

    ;; function 0
    "\08\00"
    "\41\00"                   ;; i32.const 0
    "\40"                      ;; memory.grow
    "\80\00"                   ;; memory.grow reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

;; Same as above for 3, 4, and 5-byte zero encodings.
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0b\01"                ;; Code section

    ;; function 0
    "\09\00"
    "\41\00"                   ;; i32.const 0
    "\40"                      ;; memory.grow
    "\80\80\00"                ;; memory.grow reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0c\01"                ;; Code section

    ;; function 0
    "\0a\00"
    "\41\00"                   ;; i32.const 0
    "\40"                      ;; memory.grow
    "\80\80\80\00"             ;; memory.grow reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0d\01"                ;; Code section

    ;; function 0
    "\0b\00"
    "\41\00"                   ;; i32.const 0
    "\40"                      ;; memory.grow
    "\80\80\80\80\00"          ;; memory.grow reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\07\01"                ;; Code section

    ;; function 0
    "\05\00"
    "\3f"                      ;; memory.size
    "\01"                      ;; memory.size reserved byte is not equal to zero!
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

;; memory.size reserved byte should not be a "long" LEB128 zero.
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\08\01"                ;; Code section

    ;; function 0
    "\06\00"
    "\3f"                      ;; memory.size
    "\80\00"                   ;; memory.size reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\09\01"                ;; Code section

    ;; function 0
    "\07\00"
    "\3f"                      ;; memory.size
    "\80\80\00"                ;; memory.size reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)

(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0a\01"                ;; Code section

    ;; function 0
    "\08\00"
    "\3f"                      ;; memory.size
    "\80\80\80\00"             ;; memory.size reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\05\03\01\00\00"          ;; Memory section
    "\0a\0b\01"                ;; Code section

    ;; function 0
    "\09\00"
    "\3f"                      ;; memory.size
    "\80\80\80\80\00"          ;; memory.size reserved byte
    "\1a"                      ;; drop
    "\0b"                      ;; end
  )
  "zero flag expected"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"       ;; Type section
    "\03\02\01\00"             ;; Function section
    "\0a\0c\01"                ;; Code section

    ;; function 0
    "\0a\02"
    "\ff\ff\ff\ff\0f\7f"       ;; 0xFFFFFFFF i32
    "\02\7e"                   ;; 0x00000002 i64
    "\0b"                      ;; end
  )
  "too many locals"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"  ;; Type section
    "\03\03\02\00\00"     ;; Function section with 2 functions
  )
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\0a\04\01\02\00\0b"  ;; Code section with 1 empty function
  )
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"  ;; Type section
    "\03\03\02\00\00"     ;; Function section with 2 functions
    "\0a\04\01\02\00\0b"  ;; Code section with 1 empty function
  )
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary
    "\00asm" "\01\00\00\00"
    "\01\04\01\60\00\00"           ;; Type section
    "\03\02\01\00"                 ;; Function section with 1 function
    "\0a\07\02\02\00\0b\02\00\0b"  ;; Code section with 2 empty functions
  )
  "function and code section have inconsistent lengths"
)
(module binary "\00asm\01\00\00\00\03\01\00")
(module binary "\00asm\01\00\00\00\0a\01\00"  ;; Code section with 0 functions
)
