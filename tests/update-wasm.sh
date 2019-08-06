#!/bin/sh

./host_functions_tests_1.py > host_functions_tests_1.wast
wat2wasm host_functions_tests_1.wast
xxd -i host_functions_tests_1.wasm > host_functions_tests_1.wasm.hpp
