# #############
# Defines unit tests, spec tests and fuzz tests for eos-vm.
# ############
option(ENABLE_TESTS "enable building of unit tests, spec tests." ON)
cmake_dependent_option(ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON)

configure_file(wasm_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/wasm_config.hpp)
include_directories(${CMAKE_SOURCE_DIR}/external/Catch2/single_include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
#add_subdirectory(memory_tests)
if (ENABLE_FUZZ_TESTS)
   add_subdirectory(fuzz)
endif()

enable_testing()

#file(GLOB UNIT_TESTS "i32_tests.cpp" "i64_tests.cpp" "f32_tests.cpp" "spec_tests.cpp" "varint_tests.cpp" "variant_tests.cpp" )

# ##############
# disable catch2 from installing it's own signal handlers
# ##############
add_definitions(-DCATCH_CONFIG_NO_POSIX_SIGNALS)

# ##############
# define spec tests
# ##############
#file(GLOB SPEC_TESTS "spec_tests/*.cpp" )
file(GLOB SPEC_TESTS "spec/*.cpp" )
add_executable( eos_vm_spec_tests main.cpp ${SPEC_TESTS} )
target_link_libraries( eos_vm_spec_tests eos-vm Catch2::Catch2 )

target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_SOURCE_DIR}/tests )
target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_SOURCE_DIR}/include )
target_include_directories( eos_vm_spec_tests PUBLIC ${CMAKE_BINARY_DIR}/tests/spec )

add_executable(varint_tests main.cpp varint_tests.cpp)
target_link_libraries(varint_tests eos-vm Catch2::Catch2)

add_executable( spec_test_generator ${CMAKE_CURRENT_SOURCE_DIR}/spec_tests/spec_test_generator/spec_test_generator.cpp )

catch_discover_tests( eos_vm_spec_tests )
