#ifndef FFMPEG_WRAPPER_ENCODE_H
#define FFMPEG_WRAPPER_ENCODE_H

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <string>
#include <vector>

#include "av_frame.h"
#include "av_packet.h"

/**
 * @class encode
 * @brief A wrapper class for FFmpeg encoder
 *
 * The `encode` class provides a simple wrapper around FFmpeg's encoding functionality.
 * It allows the user to create an encoder instance by either the encoder name or the encoder ID.
 */
class encode {
  AVCodec* codec_;       ///< Pointer to FFmpeg codec
  AVCodecContext* ctx_;  ///< Pointer to the encoding context

  /**
   * @brief Default constructor
   *
   * @note This constructor is private and used only for initialization in other constructors.
   */
  encode() noexcept;

 public:
  /**
   * @brief Constructor that creates an encoder instance using the encoder's name
   *
   * @param encoder_name The name of the encoder
   * @throws std::terminate If the encoder is not found or if the context cannot be allocated
   */
  explicit encode(const std::string& encoder_name) noexcept;

  /**
   * @brief Constructor that creates an encoder instance using the encoder's ID
   *
   * @param id The encoder's ID
   * @param using_hardware Whether to use hardware encoding
   * @throws std::terminate If the encoder is not found or if the context cannot be allocated
   */
  explicit encode(enum AVCodecID id, bool using_hardware = false) noexcept;

  /**
   * @brief Destructor
   *
   * Frees the encoder context resources.
   */
  ~encode();

 private:
  /**
   * @brief Sets the encoding parameters
   *
   * @param params Pointer to an AVCodecContext structure containing the encoding parameters
   */
  void set_parameters(const AVCodecContext* params) noexcept;

 public:
  /**
   * @brief Creates the encoder and opens it
   *
   * @param params Optional pointer to an AVCodecContext structure containing the encoding parameters
   */
  void create_encoder(const AVCodecContext* params = nullptr);

  /**
   * @brief Encodes a frame and returns the resulting packets
   *
   * @param frame The frame to be encoded
   * @return A vector of encoded packets
   */
  [[nodiscard]] auto encoding(av_frame frame) const -> std::vector<av_packet>;
};

#endif  // FFMPEG_WRAPPER_ENCODE_H
