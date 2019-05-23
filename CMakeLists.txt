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
option(ENABLE_SOFTFLOAT "enable the backend to use deterministic software floating point operations" ON)
option(ENABLE_MEMORY_OPS_ALIGNMENT "enable the backend to obey alignment hints" OFF)
option(ENABLE_TESTS "enable building of unit tests, spec tests." ON)
cmake_dependent_option(ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON)

 
# ############
# Create the eos-vm library.
# ############
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

if(ENABLE_SOFTFLOAT)
   if(NOT TARGET softfloat)
      add_subdirectory(external)
   endif()
   add_definitions(-DEOS_VM_SOFTFLOAT)
   target_link_libraries(eos-vm INTERFACE softfloat)
endif()


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
   set(CATCH_BUILD_TESTING OFF)
   set(CATCH_INSTALL_DOCS OFF)
   set(CATCH_INSTALL_HELPERS OFF)
   add_subdirectory(${CMAKE_SOURCE_DIR}/external/Catch2)
   include(external/Catch2/contrib/Catch.cmake)
   add_subdirectory(tests)
endif()
