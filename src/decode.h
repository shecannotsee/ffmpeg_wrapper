#ifndef FFMPEG_WRAPPER_DECODE_H
#define FFMPEG_WRAPPER_DECODE_H

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <string>
#include <vector>

#include "av_frame.h"
#include "av_packet.h"

/**
 * @class decode
 * @brief A wrapper class for FFmpeg decoders.
 *
 * The `decode` class provides a simple wrapper for FFmpeg decoders, allowing users to create a decoder instance
 * either by decoder name or by decoder ID.
 */
class decode {
  AVCodec* codec_;       ///< The FFmpeg codec
  AVCodecContext* ctx_;  ///< Codec context

  /**
   * @brief Default constructor.
   *
   * @note This constructor is private and only used for initialization in other constructors.
   */
  decode() noexcept;

 public:
  /**
   * @brief Constructs a decoder instance using the decoder name.
   *
   * @param decoder_name The name of the decoder.
   * @throws std::runtime_error If the decoder is not found or if context allocation fails.
   */
  explicit decode(const std::string& decoder_name) noexcept;

  /**
   * @brief Constructs a decoder instance using the decoder ID.
   *
   * @param id The decoder's ID.
   * @param using_hardware Whether to use hardware decoding.
   * @throws std::runtime_error If the decoder is not found or if context allocation fails.
   */
  explicit decode(enum AVCodecID id, bool using_hardware = false) noexcept;

  /**
   * @brief Destructor.
   *
   * Releases resources associated with the decoder context.
   */
  ~decode();

 private:
  /**
   * @brief Set the parameters for the codec context.
   *
   * This method sets codec context parameters based on the provided codec context.
   *
   * @param params The codec context parameters.
   */
  void set_parameters(const AVCodecContext* params) noexcept;

 public:
  /**
   * @brief Creates a decoder based on the given codec context parameters.
   *
   * @param params The codec context parameters.
   */
  void create_decoder(const AVCodecContext* params);

  /**
   * @brief Decodes a packet and returns the decoded frames.
   *
   * @param pkt The packet to decode.
   * @return A list of decoded frames.
   */
  [[nodiscard]] auto decoding(av_packet pkt) const -> std::vector<av_frame>;
};

#endif  // FFMPEG_WRAPPER_DECODE_H
