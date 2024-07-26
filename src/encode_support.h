#ifndef ENCODE_SUPPORT_H
#define ENCODE_SUPPORT_H
#include <gsl/util>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

template <typename find_type>
class encoding {
  AVCodec* codec_;             ///< decoder
  AVCodecContext* codec_ctx_;  ///< The context of decoder

 public:
  encoding() = delete;

  explicit encoding(find_type codec_name) {
    // Check if CodecType is AVCodecID or std::string
    if constexpr (std::is_same_v<find_type, AVCodecID>) {
      codec_ = avcodec_find_encoder(codec_name);
    } else if constexpr (std::is_same_v<find_type, std::string>) {
      codec_ = avcodec_find_encoder_by_name(codec_name.c_str());
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

 private:
  /**
   * @brief 通过帧信息设置编码参数
   * @param frame 原始帧信息
   */
  void set_default_encode_param(const AVFrame* frame) const {
    // Set codec parameters (you may need to adjust these based on your frames)
    codec_ctx_->bit_rate     = 400000;
    codec_ctx_->width        = frame->width;
    codec_ctx_->height       = frame->height;
    codec_ctx_->time_base    = (AVRational){1, 25};  // Assuming 25 fps
    codec_ctx_->framerate    = (AVRational){25, 1};
    codec_ctx_->gop_size     = 10;
    codec_ctx_->max_b_frames = 1;
    codec_ctx_->pix_fmt      = AV_PIX_FMT_YUV420P;
  }

  /**
   * @brief 该接口使用默认的编码上下文来创建编码器
   * @param frame 原始帧信息
   */
  void create_encoder(AVFrame* frame) const {
    this->set_default_encode_param(frame);
    if (const auto ret = avcodec_open2(codec_ctx_, codec_, nullptr); ret < 0) {
      throw std::runtime_error("Failed to open codec");
    }
  }

  /**
   * @brief 该接口使用外部编码上下文来创建编码器
   * @param codec_ctx 外部的编码上下文,里面包含了原始图像帧的信息
   */
  void create_encoder(AVCodecContext* codec_ctx) const {
    if (const auto ret = avcodec_open2(codec_ctx, codec_, nullptr); ret < 0) {
      throw std::runtime_error("Failed to open codec");
    }
  }

 public:
  /**
   * @brief 对图像帧进行编码
   * @param frames 原始图像帧
   * @param extradata SPS和PPS数据
   * @param codec_ctx 编码的上下文,主要用来控制编码算法以及相关信息,若不指定则调用默认接口进行初始化
   * @return 编码后的数据
   */
  auto get_encode_packets(const std::vector<AVFrame*>& frames,
                          std::vector<uint8_t> extradata,
                          AVCodecContext* codec_ctx = nullptr) -> std::vector<AVPacket*> {
    AVCodecContext* use_codec_ctx = nullptr;
    if (codec_ctx == nullptr) {  // 使用默认的编码格式信息
      if (frames.empty()) {
        return {};
      }
      this->create_encoder(frames[0]);
      use_codec_ctx = codec_ctx_;
      auto ret      = av_frame_get_buffer(frames[0], 0);
      if (ret < 0) {
        throw std::runtime_error("Could not allocate the video frame data");
      }
    } else {  // 使用外部设置的编码格式信息
      this->create_encoder(codec_ctx);
      use_codec_ctx = codec_ctx;
      auto ret      = av_frame_get_buffer(frames[0], 0);
      if (ret < 0) {
        throw std::runtime_error("Could not allocate the video frame data");
      }
    }

    use_codec_ctx->extradata = static_cast<uint8_t*>(av_malloc(extradata.size()));
    if (!use_codec_ctx->extradata) {
      throw std::runtime_error("Failed to allocate memory for extradata.");
    }
    memcpy(use_codec_ctx->extradata, extradata.data(), extradata.size());
    use_codec_ctx->extradata_size = extradata.size();

    std::vector<AVPacket*> packets{};
    // 开始编码
    AVPacket* packet;
    auto packet_release = gsl::finally([&]() { av_packet_free(&packet); });
    packet              = av_packet_alloc();

    for (const auto frame : frames) {
      auto ret = avcodec_send_frame(use_codec_ctx, frame);
      if (ret < 0) {
        std::string error_message = std::string("Error sending frame to encoder: ") + std::string(av_err2str(ret));
        throw std::runtime_error(error_message);
      }
      while (ret >= 0) {
        ret = avcodec_receive_packet(use_codec_ctx, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
          break;  // 一个frame已经编码完成
        } else if (ret < 0) {
          throw std::runtime_error("Error during encoding");
        }
        AVPacket* clone_packet = av_packet_alloc();
        if (av_packet_ref(clone_packet, packet) < 0) {
          fprintf(stderr, "Error while copying packet\n");
          av_packet_unref(packet);
          continue;
        }
        packets.emplace_back(clone_packet);
        av_packet_unref(packet);
      }
    }

    return packets;
  }
};

#endif  // ENCODE_SUPPORT_H
