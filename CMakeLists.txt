# ##################################################################################################
# Defines the eos-vm library and associated tooling. See docs/cmake.md for instructions on how to
# build eos-vm or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
project(eos-vm)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ##################################################################################################
# Defined options for building or integrating eos-vm.
# ##################################################################################################
include(CMakeDependentOption)
option(USE_EXISTING_BOOST "use boost (>= v1.70.0) for `outcome` library")
option(ENABLE_MEMORY_OPS_ALIGNMENT "enable the backend to obey alignment hints" OFF)
option(ENABLE_TESTS "enable building and running of unit tests" ON)
cmake_dependent_option(ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON)
option(DISABLE_CCACHE "disable building with CCache or SCCache if they are present")
option(DISABLE_GPERFTOOLS "disable building with Gperftools if it is present")
option(ENABLE_ADDRESS_SANITIZER "build with address sanitization")
option(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER "build with undefined behavior sanitization")
option(ENABLE_PROFILE "enable build for pixie or gprof")

if(NOT DISABLE_CCACHE)
   find_program(CCACHE_PROGRAM ccache)
   if(CCACHE_PROGRAM)
      message(STATUS "eos-vm using ccache")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
   endif()
endif()

if(ENABLE_PROFILE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()

if(ENABLE_ADDRESS_SANITIZER)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

if(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()

add_library(eos-vm INTERFACE)
target_include_directories(eos-vm
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                     ${CMAKE_CURRENT_SOURCE_DIR}/external/softfloat/source/include
                                     ${CMAKE_CURRENT_SOURCE_DIR}/external/outcome/single-header)

add_executable(prog ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(prog eos-vm)
target_include_directories(
   prog
   PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_CURRENT_SOURCE_DIR}/softfloat/source/include")

# add_executable( wasm_t ${CMAKE_SOURCE_DIR}/tests/fuzz/fuzz_parser.cpp ${fc} ${eos-vm}
# ${filesystem} ${chrono} ${date_time})

if(NOT TARGET softfloat)
   add_subdirectory(external)
endif()

target_link_libraries(eos-vm INTERFACE softfloat)

if(NOT USE_EXISTING_BOOST)
   add_definitions(-DEOS_VM_USE_EXTERNAL_OUTCOME)
   add_subdirectory(external/outcome)
   # else()
endif()

if(ENABLE_MEMORY_OPS_ALIGNMENT)
   add_definitions(-DEOS_VM_ALIGN_MEMORY_OPS)
endif()

if(ENABLE_TESTS)
   include(CTest)
   add_subdirectory(external/Catch2)
   include(external/Catch2/contrib/Catch.cmake)
   configure_file(tests/wasm_config.hpp.in ${CMAKE_BINARY_DIR}/tests/wasm_config.hpp)
   include_directories(external/Catch2/single_include)
   include_directories(${CMAKE_BINARY_DIR}/tests)
   add_subdirectory(memory_tests)
   add_subdirectory(tests)
   if(ENABLE_FUZZ_TESTS)
      add_subdirectory(fuzz)
   endif()
endif()
