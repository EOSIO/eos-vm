cmake_minimum_required( VERSION 3.5 )
project( EOSIO_CONTRACTS_TESTS )
set( VERSION_MAJOR 1 )
set( VERSION_MINOR 0 )
set( VERSION_PATCH 0 )

enable_testing()

#file(GLOB UNIT_TESTS "spec_tests.cpp" "f32_tests.cpp" "i32_tests.cpp" "i64_tests.cpp" ) #"*.cpp")
#file(GLOB UNIT_TESTS "i32_tests.cpp" "i64_tests.cpp" "f32_tests.cpp" "spec_tests.cpp" "varint_tests.cpp" "variant_tests.cpp" )

### disable catch2 from installing it's own signal handlers
add_definitions(-DCATCH_CONFIG_NO_POSIX_SIGNALS)
file(GLOB SPEC_TESTS "spec_tests/address_tests.cpp" )

add_subdirectory(wasms)
add_executable( eos_vm_unit_tests main.cpp ${SPEC_TESTS} )
target_link_libraries( eos_vm_unit_tests eos-vm Catch2::Catch2 )

target_include_directories( eos_vm_unit_tests PUBLIC ${CMAKE_SOURCE_DIR}/include )

catch_discover_tests( eos_vm_unit_tests )
