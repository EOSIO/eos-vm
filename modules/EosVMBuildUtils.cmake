# ##################################################################################################
# Utilities for setting up various builds.
# ##################################################################################################
include(CMakeDependentOption)

# ##################################################################################################
# Setup CCACHE
# ##################################################################################################
option(ENABLE_CCACHE "enable building with CCACHE or SCCACHE if they are present" ON)
if(NOT DISABLE_CCACHE)
   find_program(CCACHE_PROGRAM ccache)
   if(CCACHE_PROGRAM)
      message(STATUS "eos-vm using ccache")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
   endif()
endif()

# ##################################################################################################
# Setup profile builds
# ##################################################################################################
option(ENABLE_PROFILE "enable profile build" OFF)
option(ENABLE_GPERFTOOLS "enable gperftools" OFF)

# ##################################################################################################
# Setup santized builds
# ##################################################################################################
cmake_dependent_option(ENABLE_ADDRESS_SANITIZER "build with address sanitization" ON
                       "NOT ENABLE_PROFILE;NOT ENABLE_GPERFTOOLS" OFF)
cmake_dependent_option(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER
                       "build with undefined behavior sanitization" ON
                       "NOT ENABLE_PROFILE;NOT ENABLE_GPERFTOOLS" OFF)

if(ENABLE_PROFILE)
   message(STATUS "Building with profiling information.")
   compile_options("-pg")
endif()

if(ENABLE_ADDRESS_SANITIZER)
   message(STATUS "Building with address sanitization.")
   compile_options("-fsanitize=address")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

if(ENABLE_UNDEFINED_BEHAVIOR_SANITIZER)
   message(STATUS "Building with undefined behavior sanitization.")
   compile_options("-fsanitize=undefined")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()
