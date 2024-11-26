#ifndef FFMPEG_WRAPPER_DEMUX_H
#define FFMPEG_WRAPPER_DEMUX_H

extern "C" {
#include <libavformat/avformat.h>
}

#include <string>
#include <tuple>
#include <vector>

#include "av_packet.h"

struct stream_param {
  std::string key;
  std::string value;
  int flag;
};

/**
 * @brief 处理音视频解复用的类
 *
 * 该类使用 FFmpeg 库解复用音频和视频流。可以打开媒体文件，并获取指定类型的数据包。
 */
class demux {
  std::string url_;           ///< 媒体文件的 URL
  AVFormatContext* fmt_ctx_;  ///< FFmpeg 格式上下文
  struct {
    int stream_index{-1};  ///< 流索引
  } video_, audio_;

 public:
  /**
   * @brief 默认构造函数
   *
   * 构造一个 demux 对象，初始化日志记录器。
   */
  demux() noexcept;

  ~demux();

 public:
  /**
   * @brief 键值对列表类型
   *
   * 用于存储参数的键值对列表，比如[{k1,v1,0},{...},{kn,vn,0},{...}]
   */
  using key_value_list = std::vector<std::tuple<std::string, std::string, int>>;

  /**
   * @brief 打开媒体文件
   *
   * @param url 媒体文件的 URL
   * @param params 打开文件的参数
   * @throw std::runtime_error 如果无法打开文件或获取流信息
   */
  void open(const std::string& url, const key_value_list& params = {});

  /**
   * @brief 数据包类型
   */
  enum class type { audio, video, av };

  class type_av_packet {
   public:
    type t;
    av_packet pkt;
  };

  /**
   * @brief 获取指定数量的数据包
   *
   * @tparam t 数据包类型（audio, video, av）
   * @param num_packets 要获取的数据包数量
   * @return 包含数据包类型以及数据包的数组
   * @throw std::runtime_error 如果发生读取错误
   */
  template <type t>
  [[nodiscard]] auto start_receiving(size_t num_packets = 25) -> std::vector<type_av_packet>;

 public:
  /**
   * @brief 获取指定类型的编解码参数
   *
   * 根据提供的数据包类型，获取相应的编解码参数。
   *
   * @tparam t 数据包类型，支持类型包括 `type::video` 和 `type::audio`
   * @return 指向相应流的 `AVCodecParameters` 结构的指针
   * @throw std::runtime_error 如果流索引无效或超出范围
   *
   * @note 使用此方法时，请确保已成功打开媒体文件并正确设置视频和音频流索引。
   */
  template <type t>
  [[nodiscard]] auto get_codec_parameters() const -> const AVCodecParameters*;

  /**
   * @brief 获取指定类型的编解码器 ID
   *
   * 根据提供的数据包类型，获取相应流的编解码器 ID。
   *
   * @tparam t 数据包类型，支持类型包括 `type::video` 和 `type::audio`
   * @return 返回相应流的 `AVCodecID`
   * @throw std::runtime_error 如果流索引无效或超出范围
   *
   * @note 使用此方法时，请确保已成功打开媒体文件并正确设置视频和音频流索引。
   */
  template <type t>
  [[nodiscard]] auto get_codec_id() -> enum AVCodecID;

  /**
   * @brief 获取指定类型的媒体流。
   *
   * 根据模板参数类型返回对应的音频或视频流。
   *
   * @tparam t 媒体流的类型，可以是 `type::video` 或 `type::audio`。
   *
   * @return 指向 AVStream 的指针，表示请求的媒体流。
   *
   * @throws static_assert 如果模板参数类型不是 `type::video` 或 `type::audio`。
   */
  template <type t>
  [[nodiscard]] inline auto get_stream() -> AVStream*;
};

#include "demux_impl.h"

#endif  // FFMPEG_WRAPPER_DEMUX_H
