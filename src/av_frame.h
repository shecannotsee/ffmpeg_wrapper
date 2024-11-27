#ifndef FFMPEG_WRAPPER_AV_FRAME_H
#define FFMPEG_WRAPPER_AV_FRAME_H

extern "C" {
#include <libavutil/frame.h>
}

/**
 * @class av_frame
 * @brief Wrapper class for FFmpeg's AVFrame.
 *
 * This class manages the lifecycle of an AVFrame, including memory allocation and deallocation.
 */
class av_frame {
  AVFrame* frame_;  ///< Pointer to the AVFrame structure.

 public:
  /**
   * @brief Constructor that allocates an AVFrame.
   *
   * If allocation fails, a std::runtime_error exception is thrown.
   */
  av_frame() noexcept;

  /**
   * @brief Destructor that frees the allocated AVFrame.
   */
  ~av_frame();

  /**
   * @brief Copy constructor.
   *
   * This constructor creates a new av_frame object as a copy of another av_frame object.
   *
   * @param other The av_frame object to copy.
   * @throw std::runtime_error If cloning the AVFrame fails, this exception is thrown.
   */
  av_frame(const av_frame& other) noexcept;

  /**
   * @brief Move constructor.
   *
   * This constructor transfers resources from another av_frame object, avoiding unnecessary deep copies.
   *
   * @param other The av_frame object to move. After moving, the state of the original object will be invalid.
   */
  av_frame(av_frame&& other) noexcept;

  /**
   * @brief Copy assignment operator.
   *
   * This operator copies the contents of one av_frame object to the current object.
   *
   * @param other The av_frame object to copy.
   * @return A reference to the current object.
   * @throw std::runtime_error If cloning the AVFrame fails, this exception is thrown.
   */
  av_frame& operator=(const av_frame& other) noexcept;

  /**
   * @brief Move assignment operator.
   *
   * This operator transfers the resources of one av_frame object to the current object.
   *
   * @param other The av_frame object to move. After moving, the state of the original object will be invalid.
   * @return A reference to the current object.
   */
  av_frame& operator=(av_frame&& other) noexcept;

 public:
  /**
   * @brief Gets the pointer to the underlying AVFrame.
   *
   * @return A pointer to the AVFrame structure.
   */
  auto get() -> AVFrame* {
    return frame_;
  }

  /**
   * @brief Allocates a data buffer for the AVFrame.
   *
   * @param align The alignment requirement for the data, default is 32.
   * @throw std::runtime_error If allocation fails, an exception is thrown.
   */
  void allocate_buffer(int align = 32) const noexcept;

  /**
   * @brief Releases the AVFrame and its associated resources.
   *
   * Re-allocates the AVFrame. If allocation fails, a std::runtime_error exception is thrown.
   */
  void release() noexcept;

  /**
   * @brief Ensures the AVFrame is writable.
   *
   * @throw std::runtime_error If the AVFrame cannot be made writable, an exception is thrown.
   */
  void make_writable() const noexcept;
};

#endif  // FFMPEG_WRAPPER_AV_FRAME_H
