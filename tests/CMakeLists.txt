# #############
# Defines unit tests, spec tests and fuzz tests for eos-vm.
# ############
option(ENABLE_TESTS "enable building of unit tests, spec tests." ON)
cmake_dependent_option(ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON)

set(Boost_USE_STATIC_LIBS TRUE)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

configure_file(wasm_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/wasm_config.hpp)
include_directories(${CMAKE_SOURCE_DIR}/external/Catch2/single_include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#add_subdirectory(memory_tests)
if (ENABLE_FUZZ_TESTS)
   add_subdirectory(fuzz)
endif()

enable_testing()

# ##############
# disable catch2 from installing it's own signal handlers
# ##############
add_definitions(-DCATCH_CONFIG_NO_POSIX_SIGNALS)

# ##############
# define spec tests
# ##############

set( WASM_SPEC_TESTS_LOCATION "" CACHE STRING "The location of the webassembly spec tests." )
find_program(WAST2JSON wast2json)

# These tests require manual maintenance
set(SPEC_TESTS spec/elem_tests.cpp spec/func_ptrs_tests.cpp spec/globals_tests.cpp spec/memory_grow_tests.cpp spec/start_tests.cpp)

macro( add_spec_test name )
   list( APPEND SPEC_TESTS ${CMAKE_CURRENT_SOURCE_DIR}/spec/${name}_tests.cpp )
   if ( WASM_SPEC_TESTS_LOCATION )
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/spec/${name}.json
         COMMAND ${WAST2JSON} ${WASM_SPEC_TESTS_LOCATION}/${name}.wast -o ${name}.json && mv ${name}.json ${CMAKE_CURRENT_BINARY_DIR}/spec/${name}.json
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/wasms
         DEPENDS ${WASM_SPEC_TESTS_LOCATION}/${name}.wast
      )
      add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/spec/${name}_tests.cpp
         COMMAND spec_test_generator ${CMAKE_CURRENT_BINARY_DIR}/spec/${name}.json > ${CMAKE_CURRENT_SOURCE_DIR}/spec/${name}_tests.cpp
         DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/spec/${name}.json spec_test_generator)
      message("Generating: ${name}_tests.cpp")
   else()
      message("Using existing: ${name}_tests.cpp")
   endif()
endmacro()

foreach(testcase address align block br br_if br_table break-drop call_indirect
                 call const conversions f32 f32_bitwise f32_cmp f64 f64_bitwise f64_cmp
                 float_exprs float_literals float_memory float_misc forward
                 func i32 i64 if int_exprs int_literals labels left-to-right
                 load local_get local_set local_tee loop memory
                 memory_redundancy memory_size memory_trap nop return
                 stack store switch traps unreachable unwind
                 )
   add_spec_test(${testcase})
endforeach()

#file(GLOB SPEC_TESTS "spec/*.cpp")
add_executable( eos_vm_spec_tests main.cpp ${SPEC_TESTS} )
target_link_libraries( eos_vm_spec_tests eos-vm Catch2::Catch2 )

target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_SOURCE_DIR}/tests )
target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_SOURCE_DIR}/include )
target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_BINARY_DIR}/tests/spec )

add_executable(unit_tests main.cpp 
	                  varint_tests.cpp 
			  host_functions_tests.cpp 
			  signals_tests.cpp 
			  watchdog_tests.cpp)
target_link_libraries(unit_tests eos-vm Catch2::Catch2)

add_executable( spec_test_generator ${CMAKE_CURRENT_SOURCE_DIR}/spec_test_generator/spec_test_generator.cpp )

catch_discover_tests( eos_vm_spec_tests )

add_test(NAME unit_tests COMMAND unit_tests)
