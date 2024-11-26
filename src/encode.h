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
 * @brief ffmpeg 编码器封装类
 *
 * `encode` 类提供了对 ffmpeg 编码器的简单封装，允许用户通过编码器名称或编码器 ID 创建编码器实例。
 */
class encode {
  AVCodec* codec_;       ///< FFmpeg 编码器
  AVCodecContext* ctx_;  ///< 编码上下文

  /**
   * @brief 默认构造函数
   *
   * @note 此构造函数为私有，仅用于其他构造函数的初始化。
   */
  encode() noexcept;

 public:
  /**
   * @brief 使用编码器名称构造编码器实例
   *
   * @param encoder_name 编码器的名称
   * @throws std::terminate 如果未找到编码器或无法分配上下文
   */
  explicit encode(const std::string& encoder_name) noexcept;

  /**
   * @brief 使用编码器 ID 构造编码器实例
   *
   * @param id 编码器的 ID
   * @param using_hardware 是否使用硬编码器
   * @throws std::terminate 如果未找到编码器或无法分配上下文
   */
  explicit encode(enum AVCodecID id, bool using_hardware = false) noexcept;

  /**
   * @brief 析构函数
   *
   * 释放编码上下文的资源。
   */
  ~encode();

 private:
  void set_parameters(const AVCodecContext* params) noexcept;

 public:
  void create_encoder(const AVCodecContext* params = nullptr);

  auto encoding(av_frame frame) -> std::vector<av_packet>;
};

#endif  // FFMPEG_WRAPPER_ENCODE_H
