cmake_minimum_required( VERSION 3.5 )
project( EOSIO_CONTRACTS_TESTS )
set( VERSION_MAJOR 1 )
set( VERSION_MINOR 0 )
set( VERSION_PATCH 0 )

enable_testing()

file(GLOB UNIT_TESTS "spec_tests.cpp" "f32_tests.cpp" "i32_tests.cpp" "i64_tests.cpp" ) #"*.cpp")

add_subdirectory(wasms)
add_executable( eos_vm_unit_tests main.cpp ${UNIT_TESTS} )
target_link_libraries( eos_vm_unit_tests
    wasm_interpreter
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${PLATFORM_SPECIFIC_LIBS}
   )

target_include_directories( eos_vm_unit_tests PUBLIC
                            ${Boost_INCLUDE_DIRS}
                            ${CMAKE_SOURCE_DIR}/include
                            ${CMAKE_SOURCE_DIR}/fc/include )

#Manually run unit_test for all supported runtimes
#To run unit_test with all log from blockchain displayed, put --verbose after --, i.e. unit_test -- --verbose
#add_test(NAME unit_test_binaryen COMMAND unit_test
# --report_level=detailed --color_output -- --binaryen)
#add_test(NAME unit_test_wavm COMMAND unit_test
# --report_level=detailed --color_output --catch_system_errors=no -- --wavm)

if(ENABLE_COVERAGE_TESTING)

  set(Coverage_NAME ${PROJECT_NAME}_ut_coverage)

  if(NOT LCOV_PATH)
    message(FATAL_ERROR "lcov not found! Aborting...")
  endif() # NOT LCOV_PATH

  if(NOT LLVMCOV_PATH)
    message(FATAL_ERROR "llvm-cov not found! Aborting...")
  endif() # NOT LCOV_PATH

  if(NOT GENHTML_PATH)
    message(FATAL_ERROR "genhtml not found! Aborting...")
  endif() # NOT GENHTML_PATH

  # no spaces allowed within tests list
  set(ctest_tests 'unit_test_binaryen|unit_test_wavm')
  set(ctest_exclude_tests '')

  # Setup target
  add_custom_target(${Coverage_NAME}

    # Cleanup lcov
    COMMAND ${LCOV_PATH} --directory . --zerocounters

    # Run tests
    COMMAND ./tools/ctestwrapper.sh -R ${ctest_tests} -E ${ctest_exclude_tests}

    COMMAND ${LCOV_PATH} --directory . --capture --gcov-tool ./tools/llvm-gcov.sh --output-file ${Coverage_NAME}.info

    COMMAND ${LCOV_PATH} -remove ${Coverage_NAME}.info '*/boost/*' '/usr/lib/*' '/usr/include/*' '*/externals/*' '*/fc/*' '*/wasm-jit/*' --output-file ${Coverage_NAME}_filtered.info

    COMMAND ${GENHTML_PATH} -o ${Coverage_NAME} ${PROJECT_BINARY_DIR}/${Coverage_NAME}_filtered.info

    COMMAND if [ "$CI" != "true" ]\; then ${CMAKE_COMMAND} -E remove ${Coverage_NAME}.base ${Coverage_NAME}.info ${Coverage_NAME}_filtered.info ${Coverage_NAME}.total ${PROJECT_BINARY_DIR}/${Coverage_NAME}.info.cleaned ${PROJECT_BINARY_DIR}/${Coverage_NAME}_filtered.info.cleaned\; fi

    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT "Resetting code coverage counters to zero. Processing code coverage counters and generating report. Report published in ./${Coverage_NAME}"
    )

  # Show info where to find the report
  add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
    COMMAND ;
    COMMENT "Open ./${Coverage_NAME}/index.html in your browser to view the coverage report."
    )
endif()
