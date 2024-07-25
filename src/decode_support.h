#ifndef DECODE_SUPPORT_H
#define DECODE_SUPPORT_H
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class pre_decoding {
  AVFormatContext* format_ctx_;  ///< The context of input url
  std::string url_;              ///< Input data url
  int video_stream_index_;       ///< video stream index
  int audio_stream_index_;       ///< audio stream index

 public:
  pre_decoding() : video_stream_index_(-1), audio_stream_index_(-1) {
    format_ctx_ = avformat_alloc_context();
  }
  ~pre_decoding() {
    avformat_close_input(&format_ctx_);
  }

 private:
 public:
  // 从url中初始化流信息
  void set_format_from(const std::string& url) noexcept {
    url_ = url;

    auto ret = avformat_open_input(&format_ctx_, url_.c_str(), nullptr, nullptr);
    if (ret != 0) {
      throw std::runtime_error("Could not open url");
    }

    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
      throw std::runtime_error("Could not find stream information.");
    }

    // get video info
    for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
      if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        video_stream_index_ = i;
      }
    }

    // get audio info
    for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
      if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        audio_stream_index_ = i;
      }
    }
  }

  [[nodiscard]] AVCodecParameters* get_video_params() const {
    if (video_stream_index_ != -1) {
      return format_ctx_->streams[video_stream_index_]->codecpar;
    }
    throw std::runtime_error("Get video params failed");
  }

  [[nodiscard]] AVCodecID get_video_type() const {
    if (video_stream_index_ != -1) {
      return get_video_params()->codec_id;
    }
    throw std::runtime_error("Get video params failed");
  }

  void dump() const {
    av_dump_format(format_ctx_, 0, url_.c_str(), 0);
  }

  [[nodiscard]] AVFormatContext* get_stream_ctx() const {
    return format_ctx_;
  }
  [[nodiscard]] int get_video_stream_index() const {
    return video_stream_index_;
  }
};

template <typename find_type>
class decoding {
  AVCodec* codec_;             ///< decoder
  AVCodecContext* codec_ctx_;  ///< The context of decoder

  AVPacket* packet_;  ///< support decoding
  AVFrame* frame_;    ///< support decoding

 public:
  decoding() = delete;

  // Constructor taking AVCodecID or codec name as input
  explicit decoding(find_type video_type) : packet_(av_packet_alloc()), frame_(av_frame_alloc()) {
    // Check if CodecType is AVCodecID or std::string
    if constexpr (std::is_same_v<find_type, AVCodecID>) {
      codec_ = avcodec_find_decoder(video_type);
    } else if constexpr (std::is_same_v<find_type, std::string>) {
      codec_ = avcodec_find_decoder_by_name(video_type.c_str());
    } else {
      codec_ = nullptr;  // Remove compilation warnings
      static_assert(std::is_same_v<find_type, AVCodecID> || std::is_same_v<find_type, std::string>,
                    "Unsupported CodecType. Use [AVCodecID] or [std::string]");
    }

    if (!codec_) {
      throw std::runtime_error("Unsupported codec!");
    }

    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
      throw std::runtime_error("Failed to allocate codec context");
    }
  }

  ~decoding() {
    av_frame_free(&frame_);
    av_packet_free(&packet_);
    avcodec_close(codec_ctx_);
    avcodec_free_context(&codec_ctx_);
    codec_ = nullptr;
  }

 private:
 public:
  enum class state { running, done };

  void add_video_param(const AVCodecParameters* codec_parameters) const {
    const auto ret = avcodec_parameters_to_context(codec_ctx_, codec_parameters);
    if (ret < 0) {
      throw std::runtime_error("Failed to copy codec parameters to codec context");
    }
  }

  void create_decoder() const {
    const auto ret = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (ret < 0) {
      throw std::runtime_error("Failed to open codec");
    }
  }

  void set_decode_format() {
  }

  auto get_decode_frames(AVPacket* packet) -> std::tuple<state, std::vector<AVFrame*>> {
    av_packet_unref(packet_);
    packet_         = av_packet_clone(packet);
    state now_state = state::running;
    std::vector<AVFrame*> frames{};
    auto ret = avcodec_send_packet(codec_ctx_, packet_);
    if (ret < 0) {
      throw std::runtime_error("Error sending a packet for decoding");
    }

    while (ret >= 0) {
      ret = avcodec_receive_frame(codec_ctx_, frame_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        // 解码结束
        now_state = state::done;
        break;
      } else if (ret < 0) {
        throw std::runtime_error("Error during decoding");
      }
      // 拷贝AVFrame并存入vector
      AVFrame* frame_copy = av_frame_clone(frame_);
      if (!frame_copy) {
        throw std::runtime_error("Failed to clone AVFrame");
      }
      frames.push_back(frame_copy);

      // 清理当前帧，以便接收下一帧
      av_frame_unref(frame_);
    }
    return {now_state, frames};
  }

  auto get_decode_frames(AVFormatContext* format_ctx, int video_stream_index)
      -> std::tuple<state, std::vector<AVFrame*>> {
    av_packet_unref(packet_);
    state now_state = state::running;
    std::vector<AVFrame*> frames{};

    auto ret = av_read_frame(format_ctx, packet_);
    if (ret < 0) {
      now_state = state::done;
      return {now_state, frames};
    }
    // 非视频流
    if (packet_->stream_index != video_stream_index) {
      return {now_state, frames};
    }

    ret = avcodec_send_packet(codec_ctx_, packet_);
    // 错误处理
    if (ret < 0) {
      throw std::runtime_error("Error while sending a packet to the decoder");
    }

    while (ret >= 0) {
      ret = avcodec_receive_frame(codec_ctx_, frame_);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        break;
      }
      if (ret < 0) {
        throw std::runtime_error("Error while receiving a frame from the decoder");
      }
      // 拷贝AVFrame并存入vector
      AVFrame* frame_copy = av_frame_clone(frame_);
      if (!frame_copy) {
        throw std::runtime_error("Failed to clone AVFrame");
      }
      frames.push_back(frame_copy);
    }

    return {now_state,frames};
  }
};

#endif  // DECODE_SUPPORT_H
