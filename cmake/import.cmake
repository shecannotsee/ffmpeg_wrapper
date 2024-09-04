
########################################################################################################################
# Import lib                                                                                                           #
########################################################################################################################

########################################################################################################################
# import ffmpeg
set(_ffmpeg_path "/home/shecannotsee/Desktop/sheer_third_party/libraries/ffmpeg")
set(ffmpeg_include_dir "${_ffmpeg_path}/include")
set(ffmpeg_lib_dir "${_ffmpeg_path}/lib")
set(ffmpeg_link_libs avcodec avdevice avfilter avformat avutil postproc swresample swscale)
########################################################################################################################

########################################################################################################################
# import she_test
set(_she_test_path "/home/shecannotsee/Desktop/sheer_third_party/libraries/she_test")
set(she_test_include_dir "${_she_test_path}/include")
set(she_test_lib_dir)
set(she_test_link_libs)
########################################################################################################################

########################################################################################################################
# import GSL
set(_GSL_path "/home/shecannotsee/Desktop/sheer_third_party/libraries/GSL")
set(GSL_include_dir "${_GSL_path}/include")
set(GSL_lib_dir)
set(GSL_link_libs)
########################################################################################################################


# include
include_directories(${ffmpeg_include_dir}) # ffmpeg
include_directories(${she_test_include_dir}) # she_test
include_directories(${GSL_include_dir}) # gsl

# lib
link_directories(${ffmpeg_lib_dir})

# Internal project
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_SOURCE_DIR}/exec)


