#ifndef FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H
#define FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H

#include "av_frame.h"
extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

/**
 * @brief 将 NV12 格式的 AVFrame 转换为 YUV420P 格式。
 *
 * 此函数接收一个 NV12 格式的 AVFrame 作为输入，并将其转换为 YUV420P 格式，
 * 该格式在视频处理中常用。Y、Cb 和 Cr 平面将被处理以生成新的帧。
 *
 * @param nv12 输入的 NV12 格式 AVFrame。该帧将在转换完成后被替换为
 *              转换后的 YUV420P 格式帧。
 *
 * @note 输入 AVFrame 的尺寸必须与输出 AVFrame 的尺寸匹配。
 *       函数假设输入的 NV12 帧已正确分配并且具有有效的数据。
 *       同时，它还会为输出的 YUV420P 帧分配缓冲区空间。
 *
 * @warning 调用者必须确保输入的 AVFrame 不是 NULL，并且宽度和高度有效。
 *
 * @pre 输入的 AVFrame 应为 NV12 格式，并具有适当的行大小。
 *
 * @post 输入的 AVFrame 被修改为包含 YUV420P 格式的数据。
 *
 * @throw std::runtime_error 如果输入的 AVFrame 格式不是 NV12。
 */
void nv12_to_yuv420p(av_frame& nv12);

/**
 * @brief 将 YUV420P 帧转换为 NV12 帧。
 *
 * 此函数接受一个 YUV420P 帧作为输入，并将其转换为 NV12 格式。
 * 输入的 YUV420P 帧必须为 AV_PIX_FMT_YUV420P 格式。如果提供的帧
 * 格式不同，则会抛出运行时错误。
 *
 * @param[in,out] yuv420p 输入的 YUV420P 帧，将被转换后的 NV12 帧覆盖。
 *
 * @throw std::runtime_error 如果输入帧的像素格式不是
 *                            AV_PIX_FMT_YUV420P。
 *
 * 转换过程包括：
 * 1. 直接复制 Y 平面。
 * 2. 将 Cb 和 Cr 平面交错到 NV12 格式中。
 *
 * 在转换之前，会分配 NV12 帧的内存并将其标记为可写。
 */
void yuv420p_to_nv12(av_frame& yuv420p);

#endif  // FFMPEG_WRAPPER_CONVERT_AVFRAME_FORMAT_H
