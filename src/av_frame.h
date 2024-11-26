#ifndef FFMPEG_WRAPPER_AV_FRAME_H
#define FFMPEG_WRAPPER_AV_FRAME_H

extern "C" {
#include <libavutil/frame.h>
}

/**
 * @class av_frame
 * @brief FFmpeg 的 AVFrame 封装类。
 *
 * 该类管理 AVFrame 的生命周期，包括资源的分配和释放。
 */
class av_frame {
  AVFrame* frame_;  ///< 指向 AVFrame 结构体的指针。

 public:
  /**
   * @brief 构造 av_frame 对象并分配 AVFrame。
   *
   * 如果分配失败，将抛出 std::runtime_error 异常。
   */
  av_frame() noexcept;

  /**
   * @brief 析构函数，释放已分配的 AVFrame。
   */
  ~av_frame();

  /**
   * @brief 拷贝构造函数。
   *
   * 此构造函数用于创建一个新的 av_frame 对象，该对象是另一个 av_frame 对象的副本。
   *
   * @param other 要复制的 av_frame 对象。
   *
   * @throw std::runtime_error 如果 AVFrame 的克隆失败，则抛出此异常。
   */
  av_frame(const av_frame& other) noexcept;

  /**
   * @brief 移动构造函数。
   *
   * 此构造函数用于转移另一个 av_frame 对象的资源，避免不必要的深拷贝。
   *
   * @param other 要移动的 av_frame 对象。移动后，该对象的状态将被设置为无效。
   */
  av_frame(av_frame&& other) noexcept;

  /**
   * @brief 拷贝赋值操作符。
   *
   * 此操作符用于将一个 av_frame 对象的内容复制到当前对象。
   *
   * @param other 要复制的 av_frame 对象。
   *
   * @return 返回当前对象的引用。
   *
   * @throw std::runtime_error 如果 AVFrame 的克隆失败，则抛出此异常。
   */
  av_frame& operator=(const av_frame& other) noexcept;

  /**
   * @brief 移动赋值操作符。
   *
   * 此操作符用于将一个 av_frame 对象的资源转移到当前对象。
   *
   * @param other 要移动的 av_frame 对象。移动后，该对象的状态将被设置为无效。
   *
   * @return 返回当前对象的引用。
   */
  av_frame& operator=(av_frame&& other) noexcept;

 public:
  /**
   * @brief 获取底层 AVFrame 的指针。
   *
   * @return 指向 AVFrame 结构体的指针。
   */
  inline auto get() -> AVFrame* {
    return frame_;
  }

  /**
   * @brief 为 AVFrame 分配数据缓冲区。
   *
   * @param align 数据对齐要求，默认为 32。
   *
   * @throw std::runtime_error 如果分配失败。
   */
  void allocate_buffer(int align = 32) noexcept;

  /**
   * @brief 释放 AVFrame 及其相关资源。
   *
   * 重新分配 AVFrame，若分配失败，将抛出 std::runtime_error 异常。
   */
  void release() noexcept;

  /**
   * @brief 确保 AVFrame 可写。
   *
   * @throw std::runtime_error 如果无法将 AVFrame 设置为可写。
   */
  void make_writable() noexcept;
};

#endif  // FFMPEG_WRAPPER_AV_FRAME_H