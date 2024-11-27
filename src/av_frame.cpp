#include "av_frame.h"

#include <stdexcept>

av_frame::av_frame() noexcept : frame_(nullptr) {
  frame_ = av_frame_alloc();
  if (!frame_) {
    throw std::runtime_error("Failed to allocate AVPacket");
  }
}

av_frame::~av_frame() {
  if (frame_ != nullptr) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }
}

// 拷贝构造
av_frame::av_frame(const av_frame& other) noexcept : frame_(nullptr) {
  if (other.frame_) {
    frame_ = av_frame_clone(other.frame_);
    if (!frame_) {
      throw std::runtime_error("Failed to clone AVFrame");
    }
  }
}

av_frame::av_frame(av_frame&& other) noexcept : frame_(other.frame_) {
  other.frame_ = nullptr;  // Invalidate the original object
}

av_frame& av_frame::operator=(const av_frame& other) noexcept {
  if (this != &other) {
    release();  // Release current frame_
    if (other.frame_) {
      frame_ = av_frame_clone(other.frame_);
      if (!frame_) {
        throw std::runtime_error("Failed to clone AVFrame");
      }
    }
  }
  return *this;
}

av_frame& av_frame::operator=(av_frame&& other) noexcept {
  if (this != &other) {
    release();                    // Release current frame_
    frame_       = other.frame_;  // Transfer ownership
    other.frame_ = nullptr;       // Invalidate the original object
  }
  return *this;
}

void av_frame::allocate_buffer(const int align) const noexcept {
  if (const auto ret = av_frame_get_buffer(frame_, align); ret < 0) {
    throw std::runtime_error("Failed to allocate AVFrame->data");
  }
}

void av_frame::release() noexcept {
  av_frame_free(&frame_);
  frame_ = nullptr;
  frame_ = av_frame_alloc();
  if (!frame_) {
    throw std::runtime_error("Failed to allocate AVPacket when release");
  }
}

void av_frame::make_writable() const noexcept {
  const auto ret = av_frame_make_writable(frame_);
  if (ret < 0) {
    throw std::runtime_error("Failed to make AVFrame writable.");
  }
}
