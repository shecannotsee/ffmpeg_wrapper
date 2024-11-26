#ifndef FFMPEG_WRAPPER_AV_PACKET_H
#define FFMPEG_WRAPPER_AV_PACKET_H

extern "C" {
#include <libavcodec/avcodec.h>
}

/**
 * @class av_packet
 * @brief FFmpeg 的 AVPacket 封装类。
 *
 * 该类管理 AVPacket 的生命周期，包括资源的分配和释放。
 */
class av_packet {
  AVPacket* pkt_;  ///< 指向 AVPacket 结构体的指针。

 public:
  /**
   * @brief 构造 av_packet 对象并分配 AVPacket。
   *
   * 如果分配失败，将抛出 std::runtime_error 异常。
   */
  av_packet() noexcept;

  /**
   * @brief 通过原始 AVPacket 构建对象
   * @param pkt 传入指针, 构造后该指针将置为空
   *
   */
  explicit av_packet(AVPacket*& pkt) noexcept;

  /**
   * @brief 析构函数，释放已分配的 AVPacket。
   */
  ~av_packet();

  /**
   * @brief 拷贝构造函数。
   *
   * 此构造函数用于创建一个新的 av_packet 对象，该对象是另一个 av_packet 对象的副本。
   *
   * @param other 要复制的 av_packet 对象。
   *
   * @throw std::runtime_error 如果 AVPacket 的克隆失败，则抛出此异常。
   */
  av_packet(const av_packet& other) noexcept;

  /**
   * @brief 移动构造函数。
   *
   * 此构造函数用于转移另一个 av_packet 对象的资源，避免不必要的深拷贝。
   *
   * @param other 要移动的 av_packet 对象。移动后，该对象的状态将被设置为无效。
   */
  av_packet(av_packet&& other) noexcept;

  /**
   * @brief 拷贝赋值操作符。
   *
   * 此操作符用于将一个 av_packet 对象的内容复制到当前对象。
   *
   * @param other 要复制的 av_packet 对象。
   *
   * @return 返回当前对象的引用。
   *
   * @throw std::runtime_error 如果 AVPacket 的克隆失败，则抛出此异常。
   */
  av_packet& operator=(const av_packet& other) noexcept;

  /**
   * @brief 移动赋值操作符。
   *
   * 此操作符用于将一个 av_packet 对象的资源转移到当前对象。
   *
   * @param other 要移动的 av_packet 对象。移动后，该对象的状态将被设置为无效。
   *
   * @return 返回当前对象的引用。
   */
  av_packet& operator=(av_packet&& other) noexcept;

 public:
  /**
   * @brief 获取底层 AVPacket 的指针。
   *
   * @return 指向 AVPacket 结构体的指针。
   */
  inline auto get() -> AVPacket* {
    return pkt_;
  }
};

#endif  // FFMPEG_WRAPPER_AV_PACKET_H
