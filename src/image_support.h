#ifndef FFMPEG_WRAPPER_IMAGE_SUPPORT_H
#define FFMPEG_WRAPPER_IMAGE_SUPPORT_H

#include <string>

#include "av_frame.h"

/**
 * @brief Saves the given AVFrame as a PGM (Portable Gray Map) image file.
 *
 * This function takes an AVFrame object and saves its contents as a PGM image file with the specified file name.
 *
 * @param frame The AVFrame object to be saved, containing image data.
 * @param file_name The path and name of the output file.
 *
 * @note PGM (Portable Gray Map) is a simple grayscale image format.
 *       This function assumes the AVFrame object is in grayscale format and its data can be written line-by-line.
 *
 * @warning Ensure that the AVFrame object is properly initialized and contains valid image data before calling this
 * function.
 */
void pgm_save(av_frame& frame, const std::string& file_name);

/**
 * @brief Saves the given AVFrame as a JPEG file.
 *
 * This function encodes the provided AVFrame into JPEG format and saves it to the specified file.
 * Before encoding, it checks whether the pixel format of the AVFrame is compatible with MJPEG encoding.
 *
 * @param frame The AVFrame to be encoded. It must have a compatible pixel format.
 * @param file_name The file name where the JPEG image will be saved.
 *
 * @throws std::runtime_error If the frame's pixel format is unsupported, or if there is an error opening the file for
 * writing, or during the JPEG encoding process.
 *
 * Supported pixel formats:
 * - AV_PIX_FMT_YUVJ420P: Common JPEG input format
 * - AV_PIX_FMT_YUVJ422P: Suitable for certain JPEG encodings
 * - AV_PIX_FMT_YUVJ444P: High-quality JPEG encoding format
 * - AV_PIX_FMT_RGB24: Can be directly encoded into JPEG
 * - AV_PIX_FMT_RGBA: RGB format with an alpha channel (alpha channel will be discarded)
 */
void jpeg_save(av_frame& frame, const std::string& file_name);

#endif  // FFMPEG_WRAPPER_IMAGE_SUPPORT_H
