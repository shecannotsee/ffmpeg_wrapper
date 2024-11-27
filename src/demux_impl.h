#ifndef FFMPEG_WRAPPER_DEMUX_IMPL_H
#define FFMPEG_WRAPPER_DEMUX_IMPL_H

#include <stdexcept>

template <demux::type t>
[[nodiscard]] auto demux::start_receiving(const size_t num_packets) -> std::vector<type_av_packet> {
  std::vector<type_av_packet> pkt_list;

  av_packet pkt;
  auto pkt_type = type::av;
  while (pkt_list.size() < num_packets) {
    if (const auto ret = av_read_frame(fmt_ctx_, pkt.get()); ret < 0) {
      if (ret == AVERROR_EOF) {
        break;  // End of file reached, stop reading
      } else {
        // An error occurred, throw an exception
        char err[AV_ERROR_MAX_STRING_SIZE] = {0};
        const std::string err_msg =
            "Error reading frame: " + std::string(av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret));
        throw std::runtime_error(err_msg);
      }
    }

    // Filter the packet based on its type
    if constexpr (t == type::audio) {
      if (pkt.get()->stream_index != audio_.stream_index) {
        continue;  // Skip non-audio packets
      }
      pkt_type = type::audio;
    } else if constexpr (t == type::video) {
      if (pkt.get()->stream_index != video_.stream_index) {
        continue;  // Skip non-video packets
      }
      pkt_type = type::video;
    } else if constexpr (t == type::av) {
      pkt_type = pkt.get()->stream_index == audio_.stream_index ? type::audio : type::video;
    }
    // Only add valid (non-corrupted) packets to the list
    if (!(pkt.get()->flags & AV_PKT_FLAG_CORRUPT)) {
      static type_av_packet tmp;
      tmp = type_av_packet({pkt_type, pkt});
      pkt_list.emplace_back(tmp);  // Copy construct the packet
    }
    av_packet_unref(pkt.get());  // Unreference the packet to reuse the struct
  }
  return pkt_list;
}

template <demux::type t>
[[nodiscard]] auto demux::get_codec_parameters() const -> const AVCodecParameters* {
  // Check if the stream index is valid and within range
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

template <demux::type t>
[[nodiscard]] auto demux::get_codec_id() -> enum AVCodecID {
  // Retrieve the codec parameters and return the codec ID
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
    static_assert(t == type::video || t == type::audio, "Invalid type for get_stream");
  }
}

#endif  // FFMPEG_WRAPPER_DEMUX_IMPL_H
