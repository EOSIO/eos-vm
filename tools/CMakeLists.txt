# ##################################################################################################
# Create the eos-vm interpreter driver.
# ##################################################################################################
add_executable(eos-vm-interp ${CMAKE_CURRENT_SOURCE_DIR}/interp.cpp)
target_link_libraries(eos-vm-interp eos-vm)

add_executable(bench-interp ${CMAKE_CURRENT_SOURCE_DIR}/bench_interp.cpp)
target_link_libraries(bench-interp eos-vm)
