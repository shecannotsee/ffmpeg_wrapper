#include "av_packet.h"

#include <stdexcept>

av_packet::av_packet() noexcept : pkt_(nullptr) {
  pkt_ = av_packet_alloc();
  if (!pkt_) {
    throw std::runtime_error("Failed to allocate AVPacket");
  }
}

av_packet::av_packet(AVPacket*& pkt) noexcept {
  pkt_ = pkt;
  pkt  = nullptr;
}

av_packet::~av_packet() {
  if (pkt_ != nullptr) {
    av_packet_free(&pkt_);
    pkt_ = nullptr;
  }
}

av_packet::av_packet(const av_packet& other) noexcept : pkt_(nullptr) {
  if (other.pkt_) {
    pkt_ = av_packet_clone(other.pkt_);
    if (!pkt_) {
      throw std::runtime_error("Failed to clone AVPacket");
    }
  }
}

av_packet::av_packet(av_packet&& other) noexcept : pkt_(other.pkt_) {
  other.pkt_ = nullptr;  // Invalidate the original object
}

av_packet& av_packet::operator=(const av_packet& other) noexcept {
  if (this != &other) {
    av_packet_free(&pkt_);  // Free the current AVPacket
    if (other.pkt_) {
      pkt_ = av_packet_clone(other.pkt_);
      if (!pkt_) {
        throw std::runtime_error("Failed to clone AVPacket");
      }
    }
  }
  return *this;
}

av_packet& av_packet::operator=(av_packet&& other) noexcept {
  if (this != &other) {
    av_packet_free(&pkt_);    // Free the current AVPacket
    pkt_       = other.pkt_;  // Transfer ownership
    other.pkt_ = nullptr;     // Invalidate the original object
  }
  return *this;
}
