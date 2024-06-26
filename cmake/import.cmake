
########################################################################################################################
# Import lib                                                                                                           #
########################################################################################################################

# ffmpeg4.1
set(lib_path "/home/shecannotsee/Desktop/libraries")
set(ffmpeg_include ${lib_path}/ffmpeg_4.1/include)
set(ffmpeg_lib_dir ${lib_path}/ffmpeg_4.1/lib)
set(ffmpeg_lib_list -Wl,--start-group
        avcodec avfilter avformat avutil avdevice swresample swscale jpeg
        -Wl,--end-group
        z
        x264
        x265
        lzma)


# include
include_directories(${ffmpeg_include}) # ffmpeg
include_directories(${CMAKE_SOURCE_DIR}/third_party/she_test/include) # she_test
include_directories(${CMAKE_SOURCE_DIR}/third_party/gsl_4_0_0) # gsl

# lib
link_directories(${ffmpeg_lib_dir})


# Internal project
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/test)
include_directories(${CMAKE_SOURCE_DIR}/exec)


