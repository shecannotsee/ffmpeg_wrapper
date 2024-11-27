#ifndef FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H
#define FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H

#include "demux.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

/**
 * @class av_codec_context_support
 * @brief A class to support AVCodecContext management
 *
 * This class is used to manage the ffmpeg encoding context (AVCodecContext),
 * providing functionality for setting and getting encoding parameters.
 * It supports initialization, releasing, and parameter configuration of
 * the codec context, suitable for handling audio and video streams.
 */
class av_codec_context_support {
  AVCodecContext* ctx_;  ///< Pointer to the codec context

 public:
  /**
   * @brief Default constructor
   *
   * Initializes the codec context and logger.
   */
  av_codec_context_support() noexcept;

  /**
   * @brief Default destructor
   *
   * Releases the codec context and associated resources.
   */
  ~av_codec_context_support() noexcept;

 private:
  /**
   * @brief Allocates a default codec context
   *
   * This method allocates a default AVCodecContext.
   *
   * @note This method logs an error message and terminates the program if
   * the allocation fails.
   */
  void alloc_default() noexcept;

  /**
   * @brief Allocates a default codec context with a specific codec ID
   *
   * This method allocates a default AVCodecContext for a given codec ID.
   *
   * @param id The codec ID, specified by the `AVCodecID` enum for the desired encoder.
   * @throw std::terminate If the codec context allocation fails, the program terminates.
   *
   * @note This method logs an error message and terminates the program if
   * the allocation fails.
   */
  void alloc_default(enum AVCodecID id) noexcept;

  /**
   * @brief Allocates a default codec context by codec name
   *
   * This method allocates a default AVCodecContext using a specific codec name.
   *
   * @param codec_name The name of the codec.
   * @throw std::terminate If the codec context allocation fails, the program terminates.
   */
  void alloc_default(const std::string& codec_name) noexcept;

  /**
   * @brief Releases the codec context
   *
   * This method releases the current codec context to prevent memory leaks.
   */
  void release() noexcept;

 public:
  /**
   * @brief Gets the current codec context pointer
   *
   * @return A pointer to the current codec context.
   */
  [[nodiscard]] auto get() const noexcept -> const AVCodecContext* {
    return ctx_;
  }

  /**
   * @brief Adds codec parameters to the codec context
   *
   * This method retrieves codec parameters from a demux object based on
   * the specified packet type and applies them to the codec context.
   *
   * @tparam t The packet type. Supported types are `demux::type::video` and `demux::type::audio`.
   * @param stream The input demux object containing codec parameters.
   * @return A pointer to the configured codec context.
   * @throw std::runtime_error If codec parameters cannot be retrieved or other errors occur.
   */
  template <demux::type t>
  [[nodiscard]] auto add_avcodec_parameters(const demux& stream) -> const AVCodecContext* {
    release();
    alloc_default();
    auto codec_params = stream.get_codec_parameters<t>();
    avcodec_parameters_to_context(ctx_, codec_params);
    return ctx_;
  }

  /**
   * @brief Gets a codec context configured for JPEG encoding
   *
   * Initializes the codec context to support JPEG encoding and sets related parameters.
   *
   * @param width The width of the image to encode.
   * @param height The height of the image to encode.
   * @return A pointer to the configured codec context for JPEG encoding.
   */
  auto get_jpeg_encode(int width, int height) noexcept -> const AVCodecContext*;

  /**
   * @brief Gets a codec context configured for H.264 encoding using NVENC
   *
   * Initializes the codec context for H.264 encoding with NVENC hardware acceleration
   * based on the codec parameters from the demux stream.
   *
   * @param stream The demux object containing codec parameters.
   * @return A pointer to the configured codec context for H.264 NVENC encoding.
   */
  auto get_h264_nvenc_encode(const demux& stream) noexcept -> const AVCodecContext*;

  /**
   * @brief Gets a codec context configured for H.264 encoding
   *
   * Initializes the codec context for H.264 encoding based on the codec parameters
   * from the demux stream.
   *
   * @param stream The demux object containing codec parameters.
   * @return A pointer to the configured codec context for H.264 encoding.
   */
  auto get_h264_encode(const demux& stream) noexcept -> const AVCodecContext*;
};

#endif  // FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H
