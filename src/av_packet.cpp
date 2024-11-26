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

// 拷贝构造函数
av_packet::av_packet(const av_packet& other) noexcept : pkt_(nullptr) {
  if (other.pkt_) {
    pkt_ = av_packet_clone(other.pkt_);
    if (!pkt_) {
      throw std::runtime_error("Failed to clone AVPacket");
    }
  }
}

// 移动构造函数
av_packet::av_packet(av_packet&& other) noexcept : pkt_(other.pkt_) {
  other.pkt_ = nullptr;  // 使原对象失去所有权
}

// 拷贝赋值操作符
av_packet& av_packet::operator=(const av_packet& other) noexcept {
  if (this != &other) {
    av_packet_free(&pkt_);  // 释放当前的 AVPacket
    if (other.pkt_) {
      pkt_ = av_packet_clone(other.pkt_);
      if (!pkt_) {
        throw std::runtime_error("Failed to clone AVPacket");
      }
    }
  }
  return *this;
}

// 移动赋值操作符
av_packet& av_packet::operator=(av_packet&& other) noexcept {
  if (this != &other) {
    av_packet_free(&pkt_);    // 释放当前的 AVPacket
    pkt_       = other.pkt_;  // 转移所有权
    other.pkt_ = nullptr;     // 使原对象失去所有权
  }
  return *this;
}
