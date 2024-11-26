#ifndef FFMPEG_WRAPPER_DEMUX_IMPL_H
#define FFMPEG_WRAPPER_DEMUX_IMPL_H

#include <stdexcept>

template <demux::type t>
[[nodiscard]] auto demux::start_receiving(size_t num_packets) -> std::vector<type_av_packet> {
  std::vector<type_av_packet> pkt_list;

  av_packet pkt;
  type pkt_type;
  while (pkt_list.size() < num_packets) {
    auto ret = av_read_frame(fmt_ctx_, pkt.get());
    if (ret < 0) {
      if (ret == AVERROR_EOF) {
        break;
      } else {
        // 发生其他错误，抛出异常
        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
        std::string err_msg =
            "Error reading frame: " + std::string(av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret));
        throw std::runtime_error(err_msg);
      }
    }
    // 根据类型过滤数据包
    if constexpr (t == type::audio) {
      if (pkt.get()->stream_index != audio_.stream_index) {
        continue;
      }
      pkt_type = type::audio;
    } else if constexpr (t == type::video) {
      if (pkt.get()->stream_index != video_.stream_index) {
        continue;
      }
      pkt_type = type::video;
    } else if constexpr (t == type::av) {
      pkt_type = pkt.get()->stream_index == audio_.stream_index ? type::audio : type::video;
    }
    static type_av_packet tmp;
    tmp = type_av_packet({pkt_type, pkt});
    pkt_list.emplace_back(tmp);  // 拷贝构造
    av_packet_unref(pkt.get());
  }
  return pkt_list;
}

template <demux::type t>
[[nodiscard]] auto demux::get_codec_parameters() const -> const AVCodecParameters* {
  // 检查索引是否是有效值，并且在有效范围内
  if constexpr (t == type::video) {
    if (video_.stream_index < 0 || video_.stream_index >= fmt_ctx_->nb_streams) {
      throw std::runtime_error("Invalid video stream index.");
    }
    return fmt_ctx_->streams[video_.stream_index]->codecpar;
  } else if constexpr (t == type::audio) {
    if (audio_.stream_index < 0 || audio_.stream_index >= fmt_ctx_->nb_streams) {
      throw std::runtime_error("Invalid audio stream index.");
    }
    return fmt_ctx_->streams[audio_.stream_index]->codecpar;
  } else {
    static_assert(t == type::video || t == type::audio, "Invalid type for get_codec_parameters");
  }
}

template <demux::demux::type t>
[[nodiscard]] auto demux::get_codec_id() -> enum AVCodecID {
  auto codec_parameters = this->get_codec_parameters<t>();
  return codec_parameters->codec_id;
}

template <demux::type t>
[[nodiscard]] auto demux::get_stream() -> AVStream* {
  if constexpr (t == type::video) {
    return fmt_ctx_->streams[video_.stream_index];
  } else if constexpr (t == type::audio) {
    return fmt_ctx_->streams[audio_.stream_index];
  } else {
    static_assert(t == type::video || t == type::audio, "Invalid type for get_codec_parameters");
  }
}

#endif  // FFMPEG_WRAPPER_DEMUX_IMPL_H
