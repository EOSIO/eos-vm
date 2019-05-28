# ##################################################################################################
# Create the eos-vm interpreter driver.
# ##################################################################################################
add_executable(eos-vm-interp ${CMAKE_CURRENT_SOURCE_DIR}/interp.cpp)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   set(FS_LIB c++fs)
else()
   set(FS_LIB stdc++fs)
endif()
target_link_libraries(eos-vm-interp eos-vm ${FS_LIB})

add_executable(bench-interp ${CMAKE_CURRENT_SOURCE_DIR}/bench_interp.cpp)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   set(FS_LIB c++fs)
else()
   set(FS_LIB stdc++fs)
endif()
target_link_libraries(bench-interp eos-vm ${FS_LIB})
