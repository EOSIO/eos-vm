(module
  (type (;0;) (func (param i32 i32) (result i32)))
  (type (;1;) (func))
  (type (;2;) (func (result i32)))
  (type (;3;) (func (param i32) (result i32)))
  (func (;0;) (type 1))
  (func (;1;) (type 1)
    block  ;; label = @1
    end
    block  ;; label = @1
    end)
  (func (;2;) (type 2) (result i32)
    block  ;; label = @1
      nop
    end
    block (result i32)  ;; label = @1
      i32.const 7
    end)
  (func (;3;) (type 2) (result i32)
    block  ;; label = @1
      call 0
      call 0
      call 0
      call 0
    end
    block (result i32)  ;; label = @1
      call 0
      call 0
      call 0
      i32.const 8
    end)
  (func (;4;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block  ;; label = @2
        call 0
        block  ;; label = @3
        end
        nop
      end
      block (result i32)  ;; label = @2
        call 0
        i32.const 9
      end
    end)
  (func (;5;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        block (result i32)  ;; label = @3
          block (result i32)  ;; label = @4
            block (result i32)  ;; label = @5
              block (result i32)  ;; label = @6
                block (result i32)  ;; label = @7
                  block (result i32)  ;; label = @8
                    block (result i32)  ;; label = @9
                      block (result i32)  ;; label = @10
                        block (result i32)  ;; label = @11
                          block (result i32)  ;; label = @12
                            block (result i32)  ;; label = @13
                              block (result i32)  ;; label = @14
                                block (result i32)  ;; label = @15
                                  block (result i32)  ;; label = @16
                                    block (result i32)  ;; label = @17
                                      block (result i32)  ;; label = @18
                                        block (result i32)  ;; label = @19
                                          block (result i32)  ;; label = @20
                                            block (result i32)  ;; label = @21
                                              block (result i32)  ;; label = @22
                                                block (result i32)  ;; label = @23
                                                  block (result i32)  ;; label = @24
                                                    block (result i32)  ;; label = @25
                                                      block (result i32)  ;; label = @26
                                                        block (result i32)  ;; label = @27
                                                          block (result i32)  ;; label = @28
                                                            block (result i32)  ;; label = @29
                                                              block (result i32)  ;; label = @30
                                                                block (result i32)  ;; label = @31
                                                                  block (result i32)  ;; label = @32
                                                                    block (result i32)  ;; label = @33
                                                                      block (result i32)  ;; label = @34
                                                                        block (result i32)  ;; label = @35
                                                                          block (result i32)  ;; label = @36
                                                                            block (result i32)  ;; label = @37
                                                                              block (result i32)  ;; label = @38
                                                                                call 0
                                                                                i32.const 150
                                                                              end
                                                                            end
                                                                          end
                                                                        end
                                                                      end
                                                                    end
                                                                  end
                                                                end
                                                              end
                                                            end
                                                          end
                                                        end
                                                      end
                                                    end
                                                  end
                                                end
                                              end
                                            end
                                          end
                                        end
                                      end
                                    end
                                  end
                                end
                              end
                            end
                          end
                        end
                      end
                    end
                  end
                end
              end
            end
          end
        end
      end
    end)
  (func (;6;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    i32.const 2
    i32.const 3
    select)
  (func (;7;) (type 2) (result i32)
    i32.const 2
    block (result i32)  ;; label = @1
      i32.const 1
    end
    i32.const 3
    select)
  (func (;8;) (type 2) (result i32)
    i32.const 2
    i32.const 3
    block (result i32)  ;; label = @1
      i32.const 1
    end
    select)
  (func (;9;) (type 2) (result i32)
    loop (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
      call 0
      call 0
    end)
  (func (;10;) (type 2) (result i32)
    loop (result i32)  ;; label = @1
      call 0
      block (result i32)  ;; label = @2
        i32.const 1
      end
      call 0
    end)
  (func (;11;) (type 2) (result i32)
    loop (result i32)  ;; label = @1
      call 0
      call 0
      block (result i32)  ;; label = @2
        i32.const 1
      end
    end)
  (func (;12;) (type 1)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    if  ;; label = @1
      call 0
    end)
  (func (;13;) (type 2) (result i32)
    i32.const 1
    if (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
    else
      i32.const 2
    end)
  (func (;14;) (type 2) (result i32)
    i32.const 1
    if (result i32)  ;; label = @1
      i32.const 2
    else
      block (result i32)  ;; label = @2
        i32.const 1
      end
    end)
  (func (;15;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
      i32.const 2
      br_if 0 (;@1;)
    end)
  (func (;16;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 2
      block (result i32)  ;; label = @2
        i32.const 1
      end
      br_if 0 (;@1;)
    end)
  (func (;17;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
      i32.const 2
      br_table 0 (;@1;) 0 (;@1;)
    end)
  (func (;18;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 2
      block (result i32)  ;; label = @2
        i32.const 1
      end
      br_table 0 (;@1;) 0 (;@1;)
    end)
  (func (;19;) (type 0) (param i32 i32) (result i32)
    get_local 0)
  (func (;20;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
      i32.const 2
      i32.const 0
      call_indirect (type 0)
    end)
  (func (;21;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 2
      block (result i32)  ;; label = @2
        i32.const 1
      end
      i32.const 0
      call_indirect (type 0)
    end)
  (func (;22;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
      i32.const 2
      block (result i32)  ;; label = @2
        i32.const 0
      end
      call_indirect (type 0)
    end)
  (func (;23;) (type 1)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    i32.const 1
    i32.store)
  (func (;24;) (type 1)
    i32.const 10
    block (result i32)  ;; label = @1
      i32.const 1
    end
    i32.store)
  (func (;25;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    memory.grow)
  (func (;26;) (type 3) (param i32) (result i32)
    get_local 0)
  (func (;27;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    call 26)
  (func (;28;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    return)
  (func (;29;) (type 1)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    drop)
  (func (;30;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
      end
      br 0 (;@1;)
    end)
  (func (;31;) (type 2) (result i32)
    (local i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    set_local 0
    get_local 0)
  (func (;32;) (type 2) (result i32)
    (local i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    tee_local 0)
  (func (;33;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 1
    end
    set_global 0
    get_global 0)
  (func (;34;) (type 2) (result i32)
    call 23
    block (result i32)  ;; label = @1
      i32.const 1
    end
    i32.load)
  (func (;35;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      call 0
      i32.const 13
    end
    i32.ctz)
  (func (;36;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      call 0
      i32.const 3
    end
    block (result i32)  ;; label = @1
      call 0
      i32.const 4
    end
    i32.mul)
  (func (;37;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      call 0
      i32.const 13
    end
    i32.eqz)
  (func (;38;) (type 2) (result i32)
    block (result f32)  ;; label = @1
      call 0
      f32.const 0x1.8p+1 (;=3;)
    end
    block (result f32)  ;; label = @1
      call 0
      f32.const 0x1.8p+1 (;=3;)
    end
    f32.gt)
  (func (;39;) (type 2) (result i32)
    block  ;; label = @1
      br 0 (;@1;)
      unreachable
    end
    block  ;; label = @1
      i32.const 1
      br_if 0 (;@1;)
      unreachable
    end
    block  ;; label = @1
      i32.const 0
      br_table 0 (;@1;)
      unreachable
    end
    block  ;; label = @1
      i32.const 1
      br_table 0 (;@1;) 0 (;@1;) 0 (;@1;)
      unreachable
    end
    i32.const 19)
  (func (;40;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 18
      br 0 (;@1;)
      i32.const 19
    end)
  (func (;41;) (type 2) (result i32)
    block (result i32)  ;; label = @1
      i32.const 18
      br 0 (;@1;)
      i32.const 19
      br 0 (;@1;)
      i32.const 20
      i32.const 0
      br_if 0 (;@1;)
      drop
      i32.const 20
      i32.const 1
      br_if 0 (;@1;)
      drop
      i32.const 21
      br 0 (;@1;)
      i32.const 22
      i32.const 4
      br_table 0 (;@1;)
      i32.const 23
      i32.const 1
      br_table 0 (;@1;) 0 (;@1;) 0 (;@1;)
      i32.const 21
    end)
  (func (;42;) (type 2) (result i32)
    (local i32)
    i32.const 0
    set_local 0
    get_local 0
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 1
        br 1 (;@1;)
      end
    end
    i32.add
    set_local 0
    get_local 0
    block (result i32)  ;; label = @1
      block  ;; label = @2
        br 0 (;@2;)
      end
      i32.const 2
    end
    i32.add
    set_local 0
    get_local 0
    block (result i32)  ;; label = @1
      i32.const 4
      br 0 (;@1;)
      i32.ctz
    end
    i32.add
    set_local 0
    get_local 0
    block (result i32)  ;; label = @1
      block (result i32)  ;; label = @2
        i32.const 8
        br 1 (;@1;)
      end
      i32.ctz
    end
    i32.add
    set_local 0
    get_local 0)
  (func (;43;) (type 2) (result i32)
    (local i32)
    block  ;; label = @1
      i32.const 1
      set_local 0
      get_local 0
      i32.const 3
      i32.mul
      set_local 0
      get_local 0
      i32.const 5
      i32.sub
      set_local 0
      get_local 0
      i32.const 7
      i32.mul
      set_local 0
      br 0 (;@1;)
      get_local 0
      i32.const 100
      i32.mul
      set_local 0
    end
    get_local 0
    i32.const -14
    i32.eq)
  (table (;0;) 1 anyfunc)
  (memory (;0;) 1)
  (global (;0;) (mut i32) (i32.const 10))
  (export "empty" (func 1))
  (export "singular" (func 2))
  (export "multi" (func 3))
  (export "nested" (func 4))
  (export "deep" (func 5))
  (export "as-select-first" (func 6))
  (export "as-select-mid" (func 7))
  (export "as-select-last" (func 8))
  (export "as-loop-first" (func 9))
  (export "as-loop-mid" (func 10))
  (export "as-loop-last" (func 11))
  (export "as-if-condition" (func 12))
  (export "as-if-then" (func 13))
  (export "as-if-else" (func 14))
  (export "as-br_if-first" (func 15))
  (export "as-br_if-last" (func 16))
  (export "as-br_table-first" (func 17))
  (export "as-br_table-last" (func 18))
  (export "as-call_indirect-first" (func 20))
  (export "as-call_indirect-mid" (func 21))
  (export "as-call_indirect-last" (func 22))
  (export "as-store-first" (func 23))
  (export "as-store-last" (func 24))
  (export "as-memory.grow-value" (func 25))
  (export "as-call-value" (func 27))
  (export "as-return-value" (func 28))
  (export "as-drop-operand" (func 29))
  (export "as-br-value" (func 30))
  (export "as-set_local-value" (func 31))
  (export "as-tee_local-value" (func 32))
  (export "as-set_global-value" (func 33))
  (export "as-load-operand" (func 34))
  (export "as-unary-operand" (func 35))
  (export "as-binary-operand" (func 36))
  (export "as-test-operand" (func 37))
  (export "as-compare-operand" (func 38))
  (export "break-bare" (func 39))
  (export "break-value" (func 40))
  (export "break-repeated" (func 41))
  (export "break-inner" (func 42))
  (export "effects" (func 43))
  (elem (i32.const 0) 19))
