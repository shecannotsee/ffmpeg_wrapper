#ifndef FFMPEG_WRAPPER_IMAGE_SUPPORT_H
#define FFMPEG_WRAPPER_IMAGE_SUPPORT_H

#include <string>

#include "av_frame.h"

/**
 * @brief 将给定的 AVFrame 保存为 PGM 格式的图像文件。
 *
 * 此函数接收一个 AVFrame 对象，并将其内容以 PGM 格式保存到指定的文件名中。
 *
 * @param frame 要保存的 AVFrame 对象，包含图像数据。
 * @param file_name 输出文件的路径和名称。
 *
 * @note PGM (Portable Gray Map) 是一种简单的灰度图像文件格式。
 *       此函数假定 AVFrame 对象的格式是灰度格式，且数据可以按行写入文件。
 *
 * @warning 确保在调用此函数之前，AVFrame 对象已正确初始化并包含有效的图像数据。
 */
void pgm_save(av_frame& frame, const std::string& file_name);

/**
 * @brief 将给定的 AVFrame 保存为 JPEG 文件。
 *
 * 此函数将提供的 AVFrame 编码为 JPEG 格式并保存到指定文件中。
 * 在进行编码之前，它会检查 AVFrame 的像素格式是否与 MJPEG 编码器兼容。
 *
 * @param frame 要编码的 AVFrame。必须是兼容的像素格式。
 * @param file_name 将要保存 JPEG 图像的文件名。
 *
 * @throws std::runtime_error 如果帧的像素格式不受支持，或在打开写入文件时发生错误，
 *                             或在写入 JPEG 数据到文件时发生错误。
 *
 * 支持的像素格式：
 * - AV_PIX_FMT_YUVJ420P: 常用的 JPEG 输入格式
 * - AV_PIX_FMT_YUVJ422P: 适用于部分 JPEG 编码的格式
 * - AV_PIX_FMT_YUVJ444P: 高质量的 JPEG 编码格式
 * - AV_PIX_FMT_RGB24: 可以直接编码为 JPEG 格式
 * - AV_PIX_FMT_RGBA: 带透明通道的 RGB 格式（透明通道会被丢弃）
 */
void jpeg_save(av_frame& frame, const std::string& file_name);

#endif  // FFMPEG_WRAPPER_IMAGE_SUPPORT_H
