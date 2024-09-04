#ifndef DECODE_SUPPORT_H
#define DECODE_SUPPORT_H
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

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
  // 解码状态:正在解码、解码完成(通常已经到流媒体的结束位置)
  enum class state { running, done };

  /**
   * @brief 为解码器添加流格式的参数,参考pre_decoding::get_video_params
   * @param codec_parameters 流格式的参数
   */
  void add_video_param(const AVCodecParameters* codec_parameters) const {
    const auto ret = avcodec_parameters_to_context(codec_ctx_, codec_parameters);
    if (ret < 0) {
      throw std::runtime_error("Failed to copy codec parameters to codec context");
    }
  }

  /**
   * @brief 创建解码器
   */
  void create_decoder() const {
    const auto ret = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (ret < 0) {
      throw std::runtime_error("Failed to open codec");
    }
  }

  /**
   * @brief 设置解码后的数据格式
   */
  void set_decode_format() {
  }

  /**
   * @brief 独立解码的接口,
   * @param extradata 在关键帧中的SPS和PPS数据
   * 可以通过AVFormatContext->streams[video_stream_index]->codecpar中获取extradata和extradata_size()数据
   * @param packets 需要解码的图像帧数据
   * @return 原始图像帧的数组
   */
  auto independent_decoder(std::vector<uint8_t> extradata, std::vector<AVPacket*> packets) -> std::vector<AVFrame*> {
    std::vector<AVFrame*> frames{};

    // 为当前的解码器上下文添加额外数据,主要是SPS(视频序列的整体特性)和PPS(具体帧的参数)信息,里面包含解码必要的信息
    codec_ctx_->extradata = static_cast<uint8_t*>(av_malloc(extradata.size()));
    if (!codec_ctx_->extradata) {
      throw std::runtime_error("Failed to allocate memory for extradata in independent_decoder.");
    }
    memcpy(codec_ctx_->extradata, extradata.data(), extradata.size());
    codec_ctx_->extradata_size = extradata.size();

    // open
    auto ret = avcodec_open2(codec_ctx_, codec_, nullptr);
    if (ret < 0) {
      throw std::runtime_error("Failed to open codec with codec context in independent_decoder");
    }

    // 解码多个packet
    for (const auto& packet : packets) {
      // 处理一个packet
      ret = avcodec_send_packet(codec_ctx_, packet);
      if (ret < 0) {
        throw std::runtime_error("Error while sending a packet to the decoder in independent_decoder");
      }
      while (avcodec_receive_frame(codec_ctx_, frame_) >= 0) {
        // 拷贝AVFrame并存入vector
        AVFrame* frame_copy = av_frame_clone(frame_);
        if (!frame_copy) {
          throw std::runtime_error("Failed to clone AVFrame");
        }
        frames.push_back(frame_copy);
      }
    }
    return frames;
  }

  /**
   * @brief 从解码器中获取解码后的数据,调用之前需要使用 add_video_param(...) 和 create_decoder() 进行初始化
   * @param format_ctx 流格式的上下文,用来从流信息中获取下一个压缩数据packet
   * @param video_stream_index 视频流的下标,防止解码到音频了
   * @return 1.是否已经解码完成; 2.原始图像帧的数组
   */
  auto get_decode_frames(AVFormatContext* format_ctx, int video_stream_index)
      -> std::tuple<state, std::vector<AVFrame*>> {
    state now_state = state::running;
    std::vector<AVFrame*> frames{};

    av_packet_unref(packet_);
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

    return {now_state, frames};
  }
};

#endif  // DECODE_SUPPORT_H
