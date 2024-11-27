#ifndef FFMPEG_WRAPPER_AV_PACKET_H
#define FFMPEG_WRAPPER_AV_PACKET_H

extern "C" {
#include <libavcodec/avcodec.h>
}

/**
 * @class av_packet
 * @brief Wrapper class for FFmpeg's AVPacket.
 *
 * This class manages the lifecycle of an AVPacket, including memory allocation and deallocation.
 */
class av_packet {
  AVPacket* pkt_;  ///< Pointer to the AVPacket structure.

 public:
  /**
   * @brief Constructor that allocates an AVPacket.
   *
   * If allocation fails, a std::runtime_error exception is thrown.
   */
  av_packet() noexcept;

  /**
   * @brief Constructs the object using an existing AVPacket.
   * @param pkt Pointer to an existing AVPacket. The pointer will be set to nullptr after the construction.
   */
  explicit av_packet(AVPacket*& pkt) noexcept;

  /**
   * @brief Destructor that frees the allocated AVPacket.
   */
  ~av_packet();

  /**
   * @brief Copy constructor.
   *
   * This constructor creates a new av_packet object as a copy of another av_packet object.
   *
   * @param other The av_packet object to copy.
   * @throw std::runtime_error If cloning the AVPacket fails, this exception is thrown.
   */
  av_packet(const av_packet& other) noexcept;

  /**
   * @brief Move constructor.
   *
   * This constructor transfers resources from another av_packet object to the current one, avoiding unnecessary deep
   * copies.
   *
   * @param other The av_packet object to move. After moving, the state of the original object will be invalid.
   */
  av_packet(av_packet&& other) noexcept;

  /**
   * @brief Copy assignment operator.
   *
   * This operator copies the contents of one av_packet object into the current object.
   *
   * @param other The av_packet object to copy.
   * @return A reference to the current object.
   * @throw std::runtime_error If cloning the AVPacket fails, this exception is thrown.
   */
  av_packet& operator=(const av_packet& other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * This operator transfers the resources of one av_packet object to the current object.
   *
   * @param other The av_packet object to move. After moving, the state of the original object will be invalid.
   * @return A reference to the current object.
   */
  av_packet& operator=(av_packet&& other) noexcept;

  /**
   * @brief Gets the pointer to the underlying AVPacket.
   *
   * @return A pointer to the AVPacket structure.
   */
  auto get() -> AVPacket* {
    return pkt_;
  }
};

#endif  // FFMPEG_WRAPPER_AV_PACKET_H
