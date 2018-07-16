# Defines wasm_interpreter library
cmake_minimum_required( VERSION 3.5 )
project ( WasmInterpreter )
set( VERSION_MAJOR 0 )
set( VERSION_MINOR 0 )
set( VERSION_PATCH 0 )

set( CMAKE_CXX_STANDARD 14 )
set( CXX_STANDARD_REQUIRED ON )

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

list( APPEND wasm_interpreter_sources ${wasm_interpreter_headers} )
add_library ( wasm_interpreter STATIC ${wasm_interpreter_sources} )
add_executable( test ${CMAKE_SOURCE_DIR}/main.cpp )
target_link_libraries( test wasm_interpreter )
target_include_directories( wasm_interpreter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
