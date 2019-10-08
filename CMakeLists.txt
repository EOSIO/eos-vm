# ##################################################################################################
# Defines the eos-vm library and associated tooling. See docs/cmake.md for instructions on how to
# build eos-vm or integrate with another system with CMake.
# ##################################################################################################
cmake_minimum_required(VERSION 3.8)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_SUFFIX rc1)
project(eos-vm VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if (VERSION_SUFFIX)
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
   set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

message(STATUS "Building eos-vm v${VERSION_FULL}...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
link_libraries(Threads::Threads)

# ##################################################################################################
# Gate against Windows builds. 
# ##################################################################################################
if (WIN32)
   message(FATAL_ERROR "EOS-VM currently only builds on Unix like OSes (Linux, Mac OSX)")
endif()

# ##################################################################################################
# Defined options for building or integrating eos-vm.
# ##################################################################################################
include(CMakeDependentOption)
option(ENABLE_SOFTFLOAT "enable the backend to use deterministic software floating point operations"
	ON)
option(FULL_DEBUG_BUILD "enables stack dumping and instruction tracing" OFF)
option(DEBUG_COMPILER "enables debugging functionality of the compiler (requires Boost)" ON)
option(ENABLE_INSTALL "enable this library to be installed" ON)
option(ENABLE_MEMORY_OPS_ALIGNMENT "enable the backend to obey alignment hints" OFF)
option(ENABLE_TOOLS "enable building of tools" ON)
option(ENABLE_TESTS "enable building of unit tests, spec tests" OFF)
cmake_dependent_option(ENABLE_FUZZ_TESTS "enable fuzz testing" OFF "ENABLE_TESTS" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")
include(EosVMBuildUtils)

# ##################################################################################################
# Create the eos-vm library.
# ##################################################################################################
add_library(eos-vm INTERFACE)
target_include_directories(eos-vm
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
                                     ${CMAKE_CURRENT_SOURCE_DIR}/external/softfloat/source/include
                                     ${CMAKE_CURRENT_SOURCE_DIR}/external/outcome/single-header)

# ##################################################################################################
# Enable debugging stats for eos-vm.
# ##################################################################################################
if(FULL_DEBUG_BUILD)
   target_compile_definitions(eos-vm INTERFACE -DEOS_VM_FULL_DEBUG)
endif()

# ##################################################################################################
# Enable compiler debugging functionality for eos-vm.
# ##################################################################################################
if(DEBUG_COMPILER)
   # need Boost for graphviz CFG dumping
   set(Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF")
   find_package(Boost 1.67 REQUIRED COMPONENTS
      graph)
   target_include_directories(eos-vm INTERFACE ${Boost_INCLUDE_DIR})
   target_link_libraries(eos-vm INTERFACE ${Boost_LIBRARIES})
   target_compile_definitions(eos-vm INTERFACE -DEOS_VM_DEBUG_COMPILER)
endif()
   

# ##################################################################################################
# Option specific build configurations.
# ##################################################################################################
if(ENABLE_SOFTFLOAT)
   if(NOT TARGET softfloat)
      add_subdirectory(external)
   endif()
   target_compile_definitions(eos-vm INTERFACE -DEOS_VM_SOFTFLOAT)
   target_link_libraries(eos-vm INTERFACE softfloat)
endif()

if(NOT USE_EXISTING_BOOST)
   target_compile_definitions(eos-vm INTERFACE -DEOS_VM_USE_EXTERNAL_OUTCOME)
   add_subdirectory(external/outcome)
endif()

if(ENABLE_MEMORY_OPS_ALIGNMENT)
   target_compile_definitions(eos-vm INTERFACE -DEOS_VM_ALIGN_MEMORY_OPS)
endif()

# ##################################################################################################
# Build eos-vm tools.
# ##################################################################################################
if(ENABLE_TOOLS)
add_subdirectory(tools)
endif()

# ##################################################################################################
# Build eos-vm tests.
# ##################################################################################################
if(ENABLE_TESTS)
   include(CTest)
   set(CATCH_BUILD_TESTING OFF CACHE BOOL "disable building tests")
   set(CATCH_INSTALL_DOCS OFF CACHE BOOL "disable installing docs")
   set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "disable installing helpers")
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/Catch2)
   include(external/Catch2/contrib/Catch.cmake)
   add_subdirectory(tests)
   if (NOT EXISTS ${CMAKE_BINARY_DIR}/wasms)
      include(ExternalProject)
      ExternalProject_Add(
         eos_vm_test_wasms
         GIT_REPOSITORY "https://github.com/EOSIO/eos-vm-test-wasms.git"
         SOURCE_DIR ${CMAKE_BINARY_DIR}/wasms
         BINARY_DIR ${CMAKE_BINARY_DIR}/wasms
         BUILD_ALWAYS 0
         BUILD_COMMAND ""
         TEST_COMMAND ""
         INSTALL_COMMAND ""
         UPDATE_COMMAND ""
         PATCH_COMMAND ""
         CONFIGURE_COMMAND ""
      )
   endif()
endif()

# ##################################################################################################
# Installation.
# ##################################################################################################
if(ENABLE_INSTALL)
   include(GNUInstallDirs)
   message(STATUS "Installing...")
   install(TARGETS eos-vm
            LIBRARY
               DESTINATION ${CMAKE_INSTALL_LIBDIR}
               COMPONENT Libraries
            PUBLIC_HEADER
               DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
               COMPONENT Headers)

   install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/eosio
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

