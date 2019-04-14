cmake_minimum_required( VERSION 3.5 )
project( FUZZ_TESTS )

file(GLOB FUZZ_TESTS "*.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer")
add_executable( fuzz_test ${FUZZ_TESTS} )
target_link_libraries( fuzz_test 
    fc
    wasm_interpreter
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${PLATFORM_SPECIFIC_LIBS}
   )

target_include_directories( fuzz_test PUBLIC
                            ${Boost_INCLUDE_DIRS}
                            ${CMAKE_SOURCE_DIR}/include
                            ${CMAKE_SOURCE_DIR}/fc/include )
