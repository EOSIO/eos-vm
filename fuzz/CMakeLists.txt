set(CMAKE_CXX_FLAGS "-g -O1 -fsanitize-coverage=trace-pc-guard,indirect-calls,trace-cmp,trace-div,trace-gep -gline-tables-only")

find_package( LLVM REQUIRED CONFIG )
message( STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}" )
message( STATUS "Using LLVMConfig.cmake at: ${LLVM_DIR}" )

function(add_libfuzzer_test name)
 set(multi_arg_options "SOURCES")
 cmake_parse_arguments(
     "add_libfuzzer_test" "" "" "${multi_arg_options}" ${ARGN})
 if ("${add_libfuzzer_test_SOURCES}" STREQUAL "")
     message(FATAL_ERROR "Source files must be specified")
 endif()
 add_executable(LLVMFuzzer-${name}
    ${add_libfuzzer_test_SOURCES}
 )
 target_compile_options(LLVMFuzzer-${name} PUBLIC "-fsanitize=fuzzer,address")
 target_link_libraries(LLVMFuzzer-${name} PUBLIC eos-vm "-fsanitize=fuzzer,address")
endfunction()

add_libfuzzer_test(wasm_fuzz_test SOURCES fuzz_driver.cpp)

add_test(wasm_libfuzzer COMMAND ${CMAKE_CURRENT_BINARY_DIR}/LLVMFuzzer-wasm_fuzz_test -jobs=40 ${CMAKE_CURRENT_SOURCE_DIR}/corpus)
