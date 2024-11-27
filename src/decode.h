#ifndef FFMPEG_WRAPPER_DECODE_H
#define FFMPEG_WRAPPER_DECODE_H

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <string>
#include <vector>

#include "av_frame.h"
#include "av_packet.h"

/**
 * @class decode
 * @brief ffmpeg 解码器封装类
 *
 * `decode` 类提供了对 ffmpeg 编码器的简单封装，允许用户通过编码器名称或解码器 ID 创建解码器实例。
 */
class decode {
  AVCodec* codec_;       ///< FFmpeg 编码器
  AVCodecContext* ctx_;  ///< 编码上下文

  /**
   * @brief 默认构造函数
   *
   * @note 此构造函数为私有，仅用于其他构造函数的初始化。
   */
  decode() noexcept;

 public:
  /**
   * @brief 使用解码器名称构造解码器实例
   *
   * @param decoder_name 解码器的名称
   * @throws std::terminate 如果未找到解码器或无法分配上下文
   */
  explicit decode(const std::string& decoder_name) noexcept;

  /**
   * @brief 使用解码器 ID 构造解码器实例
   *
   * @param id 解码器的 ID
   * @param using_hardware 是否使用硬解码器
   * @throws std::terminate 如果未找到解码器或无法分配上下文
   */
  explicit decode(enum AVCodecID id, bool using_hardware = false) noexcept;

  /**
   * @brief 析构函数
   *
   * 释放解码上下文的资源。
   */
  ~decode();

 private:
  void set_parameters(const AVCodecContext* params) noexcept;

 public:
  void create_decoder(const AVCodecContext* params);

  [[nodiscard]] auto decoding(av_packet pkt)const -> std::vector<av_frame>;
};

#endif  // FFMPEG_WRAPPER_DECODE_H
