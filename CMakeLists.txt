# Defines eos-vm library
cmake_minimum_required( VERSION 3.8 )
project ( eos-vm )
set( VERSION_MAJOR 0 )
set( VERSION_MINOR 0 )
set( VERSION_PATCH 0 )

set( CMAKE_CXX_STANDARD 17 )

include(CMakeDependentOption)

option(USE_EXISITING_SOFTFLOAT "use exisiting softfloat library")
option(ENABLE_TESTS "enable building and running of unit tests" ON)
CMAKE_DEPENDENT_OPTION( ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON )
option(DISABLE_CCACHE "disable building with CCache or SCCache if they are present")
option(DISABLE_GPERFTOOLS "disable building with Gperftools if it is present")
option(ENABLE_ADDRESS_SANITIZER "build with address sanitization")
option(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER "build with undefined behavior sanitization")
option(ENABLE_PROFILE "enable build for pixie or gprof")



#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
#find_package(Threads)
#link_libraries(Threads::Threads)

if (NOT DISABLE_CCACHE)
   find_program(CCACHE_PROGRAM ccache)
   if (CCACHE_PROGRAM)
      message(STATUS "EOS-VM found ccache")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
   endif()
endif()

if (ENABLE_PROFILE)
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg" )
endif()

if (ENABLE_ADDRESS_SANITIZER)
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" )
   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address" )
endif()

if (ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined" )
   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined" )
endif()

#FIND_PACKAGE(Boost 1.50 REQUIRED COMPONENTS
#    thread
#    date_time
#    filesystem
#    system
#    program_options
#    signals
#    serialization
#    chrono
#    unit_test_framework
#    context
#    locale
#    iostreams)

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -O3 -std=c++17" )

# generate compile command database for external tools
set (CMAKE_EXPORT_COMPILE_COMMANDS "ON")

#message( STATUS "Configuring WasmInterpreter" )
#set( CMAKE_C_FLAGS " -Wall -Werror-implicit-function-declaration -Wno-conversion ${CMAKE_C_FLAGS} ${C_DEFINES}" )

file( GLOB eos_vm_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/eosio/wasm_backend/*.hpp" )

add_library( eos-vm INTERFACE )
target_include_directories( eos-vm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/external/softfloat/source/include )

add_executable( prog ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp )
target_link_libraries( prog eos-vm )
target_include_directories( prog PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_CURRENT_SOURCE_DIR}/softfloat/source/include")

#add_executable( wasm_t ${CMAKE_SOURCE_DIR}/tests/fuzz/fuzz_parser.cpp ${fc} ${eos-vm} ${filesystem} ${chrono} ${date_time})

#set(USE_EXISTING_SOFTFLOAT "True")
if (NOT USE_EXISTING_SOFTFLOAT)
   add_subdirectory( external )
   target_link_libraries( eos-vm INTERFACE sf )
else()
   target_link_libraries( eos-vm INTERFACE softfloat )
endif()

if (ENABLE_TESTS)
   #add_subdirectory(tests)
   include(CTest)
   add_subdirectory(external/Catch2)
   include(external/Catch2/contrib/Catch.cmake)
   include_directories(external/Catch2/single_include)
   add_subdirectory(memory_tests)
   if (ENABLE_FUZZ_TESTS)
      add_subdirectory(fuzz)
   endif()
   #add_subdirectory(tests/fuzz)
endif()
