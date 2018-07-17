# Defines wasm_interpreter library
cmake_minimum_required( VERSION 3.5 )
project ( WasmInterpreter )
set( VERSION_MAJOR 0 )
set( VERSION_MINOR 0 )
set( VERSION_PATCH 0 )

set( CMAKE_CXX_STANDARD 14 )
set( CXX_STANDARD_REQUIRED ON )

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling tests with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
IF( WIN32 )
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  set(Boost_USE_MULTITHREADED ON)
  set(BOOST_ALL_DYN_LINK OFF) # force dynamic linking for all libraries
ENDIF(WIN32)
FIND_PACKAGE(Boost 1.67 REQUIRED COMPONENTS
    thread
    date_time
    filesystem
    system
    program_options
    signals
    serialization
    chrono
    unit_test_framework
    context
    locale
    iostreams)


# add defaults for openssl
if ("${OPENSSL_ROOT_DIR}" STREQUAL "")
   if (NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
      set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR})
      set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
   elseif (APPLE)
      set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
      set(OPENSSL_INCLUDE_DIR "/usr/local/opt/openssl/include")
   elseif(UNIX AND NOT APPLE)
      set(OPENSSL_ROOT_DIR "/usr/include/openssl")
      set(OPENSSL_INCLUDE_DIR "/usr/include/openssl/include")
   else()
      message(FATAL_ERROR "openssl not found and don't know where to look, please specify OPENSSL_ROOT_DIR")
   endif()
endif()

if ( APPLE )
   set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-deprecated-declarations" )
else()
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc" )
endif()

# generate compile command database for external tools
set (CMAKE_EXPORT_COMPILE_COMMANDS "ON")

message( STATUS "Configuring WasmInterpreter" )
set( CMAKE_C_FLAGS " -Wall -Werror-implicit-function-declaration -Wno-conversion ${CMAKE_C_FLAGS} ${C_DEFINES}" )

file( GLOB wasm_interpreter_sources "${CMAKE_SOURCE_DIR}/src/*.cpp" )
file( GLOB wasm_interpreter_headers "${CMAKE_SOURCE_DIR}/include/eosio/wasm_backend/*.hpp" )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/fc/CMakeModules" )
add_subdirectory(fc)

list( APPEND wasm_interpreter_sources ${wasm_interpreter_headers} )
add_library ( wasm_interpreter STATIC ${wasm_interpreter_sources} )
add_executable( test ${CMAKE_SOURCE_DIR}/main.cpp )

target_link_libraries( test wasm_interpreter fc )
target_include_directories( wasm_interpreter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                                    "${CMAKE_SOURCE_DIR}/fc/include" )

add_subdirectory(tests)
