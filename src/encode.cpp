#include "encode.h"

#include <stdexcept>

extern "C" {
#include <libavutil/opt.h>
}

encode::encode() noexcept : codec_(nullptr), ctx_(nullptr) {
}

encode::encode(const std::string& encoder_name) noexcept : encode() {
  codec_ = avcodec_find_encoder_by_name(encoder_name.c_str());
  if (!codec_) {
    throw std::runtime_error("Codec " + encoder_name + " not found.");
  }

  ctx_ = avcodec_alloc_context3(codec_);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context");
  }
}

encode::encode(enum AVCodecID id, bool using_hardware) noexcept : encode() {
  if (using_hardware) {
    if (id != AV_CODEC_ID_H264 && id != AV_CODEC_ID_HEVC) {
      throw std::runtime_error("Unsupported hard encoding format: " +
                               std::string(avcodec_get_name(id) ? avcodec_get_name(id) : "Unknown codec"));
    }
    std::string hard_encoder_name = (id == AV_CODEC_ID_H264) ? "h264_nvenc" : "hevc_nvenc";
    //
    codec_ = avcodec_find_encoder_by_name(hard_encoder_name.c_str());
    if (!codec_) {
      throw std::runtime_error("Codec " + hard_encoder_name + " not found.");
    }
  } else {
    codec_ = avcodec_find_encoder(id);
    if (!codec_) {
      throw std::runtime_error("Codec " + std::string(avcodec_get_name(id) ? avcodec_get_name(id) : "Unknown codec") +
                               " not found.");
    }
  }

  ctx_ = avcodec_alloc_context3(codec_);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context");
  }
}

encode::~encode() {
  avcodec_free_context(&ctx_);
  ctx_   = nullptr;
  codec_ = nullptr;
}

void encode::set_parameters(const AVCodecContext* params) noexcept {
  constexpr int default_bit_rate     = 40000;
  constexpr int default_width        = 1920;
  constexpr int default_height       = 1080;
  constexpr auto default_time_base   = (AVRational){1, 25};
  constexpr auto default_framerate   = (AVRational){25, 1};
  constexpr int default_gop_size     = 10;
  constexpr int default_max_b_frames = 0;
  if (params) {
    // Encoding with custom parameters...
    ctx_->bit_rate     = params->bit_rate;
    ctx_->width        = params->width;
    ctx_->height       = params->height;
    ctx_->time_base    = params->time_base;
    ctx_->framerate    = params->framerate;
    ctx_->gop_size     = params->gop_size;
    ctx_->max_b_frames = params->max_b_frames;
    ctx_->pix_fmt      = params->pix_fmt;
    // 设置其他参数
  } else {
    // Encoding using default parameters...
    /* put sample parameters */
    ctx_->bit_rate = default_bit_rate;  // 默认比特率
    ctx_->width    = default_width;     // 默认宽度
    ctx_->height   = default_height;    // 默认高度
    /* frames per second */
    ctx_->time_base = default_time_base;
    ctx_->framerate = default_framerate;
    // 每十帧发射一个帧内帧
    // 在传递帧之前检查帧pict_type
    // 如果帧->pict_type为AV_PICTURE_type_I，则发送到编码器
    // 则忽略gop\usize，并输出编码器
    // 无论gop_size如何，都将始终是I帧
    ctx_->gop_size     = default_gop_size;
    ctx_->max_b_frames = default_max_b_frames;
    ctx_->pix_fmt      = AV_PIX_FMT_YUVJ420P;
    if (codec_->id == AV_CODEC_ID_H264) {
      av_opt_set(ctx_->priv_data, "preset", "slow", 0);
    }
  }
}

void encode::create_encoder(const AVCodecContext* params) {
  this->set_parameters(params);

  auto ret = avcodec_open2(ctx_, codec_, nullptr);
  if (ret < 0) {
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    std::string warn_msg =
        "Could not open codec: " + std::string(av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret));
    throw std::runtime_error(warn_msg);
  }
}

auto encode::encoding(av_frame frame) -> std::vector<av_packet> {
  std::vector<av_packet> pkt_list;

  auto ret = avcodec_send_frame(ctx_, frame.get());
  if (ret < 0) {
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    throw std::runtime_error("Error sending a frame for encoding: " +
                             std::string(av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret)));
  }

  av_packet pkt;
  while (ret >= 0) {
    ret = avcodec_receive_packet(ctx_, pkt.get());
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return pkt_list;
    } else if (ret < 0) {
      throw std::runtime_error("Error during encoding: " + std::to_string(ret));
    }

    pkt_list.emplace_back(pkt);  // 拷贝构造, 不能够移动否则会导致内存安全问题
    av_packet_unref(pkt.get());
  }

  return pkt_list;
}
