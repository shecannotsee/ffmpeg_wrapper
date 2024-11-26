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

########################################################################################################################
# set ffmpeg
set(FFMPEG_ROOT_DIR "/usr/local" CACHE STRING "default path")
set(_ffmpeg_path ${FFMPEG_ROOT_DIR})
set(ffmpeg_include_dir "${_ffmpeg_path}/include")
set(ffmpeg_lib_dir "${_ffmpeg_path}/lib")
set(ffmpeg_link_libs avcodec avdevice avfilter avformat avutil postproc swresample swscale)
# import ffmpeg
include_directories(${ffmpeg_include_dir})
link_directories(${ffmpeg_lib_dir})
list(APPEND src_link_dependency ${ffmpeg_link_libs})
########################################################################################################################
