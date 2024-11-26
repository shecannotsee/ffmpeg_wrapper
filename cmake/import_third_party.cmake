# internal project
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/test)

########################################################################################################################
# set: gtest
set(_gtest_path "${CMAKE_CURRENT_SOURCE_DIR}/third_party/googletest-1.12.0")
set(gtest_include_dir "${_gtest_path}/include")
set(gtest_lib_dir "${_gtest_path}/lib")
set(gtest_link_libs gmock gtest)
# import gtest
include_directories(${gtest_include_dir})
link_directories(${gtest_lib_dir})
list(APPEND test_link_dependency ${gtest_link_libs})
########################################################################################################################


