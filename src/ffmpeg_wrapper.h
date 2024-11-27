/*!
 * \mainpage ffmpeg_wrapper documentation
 *
 * \section intro Introduction
 *
 * The `ffmpeg_wrapper` project is a lightweight C++ wrapper around FFmpeg 4.3 that simplifies the process of encoding,
 * decoding, and manipulating multimedia files.
 *
 * \section structure Project Structure
 * - [Class Documentation](classes.html): Here is detailed documentation of the project's classes.
 * - [File Documentation](files.html): Here is a list of all the files in the project and what they do.
 *
 *  \section dependencies Project Dependencies
 * - FFmpeg 4.3: The wrapper relies on FFmpeg 4.3 for multimedia processing. Please ensure you have FFmpeg installed and
 * configured correctly.
 * - Google Test (gtest): If you wish to run the tests, you need to have Google Test installed. You can install it via
 * your package manager or build it from source.
 *
 * \section usage Instructions
 * -DFFMPEG_ROOT_DIR=/home/shecannotsee/Desktop/sheer_third_party/libraries/ffmpeg-4.3
 * cmake build options：
 * - -DFFMPEG_ROOT_DIR=.../ffmpeg-4.3
 * - -Dbuild_release=ON
 * - -Dbuild_tests=OFF
 * - -build_shared_libs=ON
 * - sample: `cmake -DFFMPEG_ROOT_DIR=.../ffmpeg-4.3 -Dbuild_release=ON -Dbuild_tests=OFF -build_shared_libs=ON .`
 *
 * If you need help, refer to the specific class or file documentation, or contact the developer.
 */
#ifndef FFMPEG_WRAPPER_H
#define FFMPEG_WRAPPER_H

#include "av_codec_context_support.h"
#include "av_frame.h"
#include "av_packet.h"
#include "convert_avframe_format.h"
#include "decode.h"
#include "demux.h"
#include "draw_on_avframe.h"
#include "encode.h"
#include "frame_support.h"
#include "image_support.h"

#endif  // FFMPEG_WRAPPER_H
