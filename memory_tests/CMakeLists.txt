file (GLOB memory_tests_files *.cpp)

add_executable( memory_tests ${memory_tests_files} )
target_link_libraries( memory_tests eos-vm Catch2::Catch2 )

catch_discover_tests( memory_tests )
