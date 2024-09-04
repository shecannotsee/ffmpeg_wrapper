#ifndef STREAM_SUPPORT_H
#define STREAM_SUPPORT_H

#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include "libavformat/avformat.h"
}

struct stream_param {
  std::string key;
  std::string value;
  int flag;
};

class stream_info {
  AVFormatContext* format_ctx_;  ///< The context of input url
  std::string url_;              ///< Input data url
  int video_stream_index_;       ///< video stream index
  int audio_stream_index_;       ///< audio stream index

public:
  stream_info();

  ~stream_info();

  /**
   * @brief url中初始化流信息
   * @param url 可以是媒体文件路径,也可以是rtsp流地址
   * @param params 控制流的参数
   */
  void set_format_from(const std::string& url, const std::vector<stream_param>& params) noexcept;

  /**
   * @brief 获取视频流的一些参数,参考decoding::add_video_param
   * @return 编码器所需要的参数
   */
  [[nodiscard]] AVCodecParameters* get_video_params() const;

  /**
   * @brief 获取流的类型
   * @return 流类型,参考ffmpeg的AVCodecID枚举
   */
  [[nodiscard]] AVCodecID get_video_type() const;

  /**
   * @brief 获取流格式的上下文
   * @return 流格式的上下文
   */
  [[nodiscard]] AVFormatContext* get_stream_ctx() const;

  /**
   * @brief 获取视频流的数组下标
   * @return 数组下标
   */
  [[nodiscard]] int get_video_stream_index() const;

  void dump() const;
};

#endif  // STREAM_SUPPORT_H
