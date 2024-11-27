#ifndef FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H
#define FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H

#include "demux.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

/**
 * @brief 支持 AVCodecContext 的类
 *
 * 此类用于管理 ffmpeg 的编码上下文（AVCodecContext），提供编码参数的设置和获取功能。
 * 它支持对编解码器的初始化、释放及参数配置，适用于音视频流的处理。
 */
class av_codec_context_support {
  AVCodecContext* ctx_;  ///< 编码上下文

 public:
  /**
   * @brief 默认构造函数
   *
   * 初始化编码上下文和日志记录器。
   */
  av_codec_context_support() noexcept;

  /**
   * @brief 默认析构函数
   *
   * 释放编码上下文的资源。
   */
  ~av_codec_context_support() noexcept;

 private:
  /**
   * @brief 分配默认编码上下文
   *
   * 该方法会分配一个默认的 AVCodecContext。
   *
   * @note 此方法将在分配失败时记录错误信息，并终止程序执行。
   */
  void alloc_default() noexcept;

  /**
   * @brief 分配默认编码上下文并指定编解码器 ID
   *
   * 根据给定的编解码器 ID，分配一个默认的 AVCodecContext。
   *
   * @param id 编解码器的 ID，使用 `AVCodecID` 枚举指定所需的编码器
   * @throw std::terminate 如果分配编码上下文失败，将导致程序终止
   *
   * @note 此方法将在分配失败时记录错误信息，并终止程序执行。
   */
  void alloc_default(enum AVCodecID id) noexcept;

  void alloc_default(const std::string& codec_name) noexcept;

  /**
   * @brief 释放编码上下文
   *
   * 该方法会释放当前的编码上下文，防止内存泄漏。
   */
  void release() noexcept;

 public:
  /**
   * @brief 获取编码上下文指针
   *
   * @return 指向当前编码上下文的指针
   */
  [[nodiscard]] auto get() const noexcept -> const AVCodecContext* {
    return ctx_;
  }

  /**
   * @brief 添加编解码参数到编码上下文
   *
   * 根据指定的数据包类型，从 demux 对象中获取编解码参数，并应用到编码上下文。
   *
   * @tparam t 数据包类型，支持类型包括 `demux::type::video` 和 `demux::type::audio`
   * @param stream 输入的 demux 对象
   * @return 返回配置后的编码上下文指针
   * @throw std::runtime_error 如果无法获取编解码参数或发生其他错误
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
     * @brief 获取 JPEG 编码上下文
     *
     * 初始化编码上下文以支持 JPEG 编码，并设置相关参数。
     *
     * @param width 编码图像的宽度
     * @param height 编码图像的高度
     * @return 返回配置后的编码上下文指针
     */
  auto get_jpeg_encode(int width, int height) noexcept -> const AVCodecContext*;

  auto get_h264_nvenc_encode(const demux& stream) noexcept -> const AVCodecContext*;

  auto get_h264_encode(const demux& stream) noexcept -> const AVCodecContext*;
};

#endif  // FFMPEG_WRAPPER_AV_CODEC_CONTEXT_H