#ifndef FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H
#define FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H

#include "av_frame.h"

/**
 * @brief Converts an NV12 formatted AVFrame to YUV420P format.
 *
 * This function takes an NV12 formatted AVFrame as input and converts it to the YUV420P format,
 * which is commonly used in video processing. The Y, Cb, and Cr planes will be processed to
 * generate a new frame.
 *
 * @param nv12 The input AVFrame in NV12 format. The frame will be replaced with the
 *             converted YUV420P formatted frame after the conversion.
 *
 * @note The input AVFrame's size must match the output AVFrame's size.
 *       The function assumes the input NV12 frame is correctly allocated and has valid data.
 *       It will also allocate buffer space for the output YUV420P frame.
 *
 * @warning The caller must ensure that the input AVFrame is not NULL, and that the width and height are valid.
 *
 * @pre The input AVFrame should be in NV12 format and have appropriate strides (linesizes).
 *
 * @post The input AVFrame is modified to contain YUV420P format data.
 *
 * @throw std::runtime_error If the input AVFrame is not in NV12 format.
 */
void nv12_to_yuv420p(av_frame& nv12);

/**
 * @brief Converts a YUV420P frame to NV12 format.
 *
 * This function takes a YUV420P formatted frame as input and converts it to NV12 format.
 * The input YUV420P frame must be in AV_PIX_FMT_YUV420P format. If the provided frame
 * format is different, a runtime error will be thrown.
 *
 * @param[in,out] yuv420p The input YUV420P frame, which will be overwritten by the
 *                       converted NV12 frame.
 *
 * @throw std::runtime_error If the input frame's pixel format is not
 *                            AV_PIX_FMT_YUV420P.
 *
 * The conversion process involves:
 * 1. Directly copying the Y plane.
 * 2. Interleaving the Cb and Cr planes into the NV12 format.
 *
 * Before conversion, memory for the NV12 frame will be allocated and marked as writable.
 */
void yuv420p_to_nv12(av_frame& yuv420p);

#endif  // FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H
