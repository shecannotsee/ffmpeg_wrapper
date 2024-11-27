#include "decode.h"

extern "C" {
#include <libavutil/opt.h>
}

#include <stdexcept>

decode::decode() noexcept : codec_(nullptr), ctx_(nullptr) {
}

decode::decode(const std::string& decoder_name) noexcept : decode() {
  codec_ = avcodec_find_decoder_by_name(decoder_name.c_str());
  if (!codec_) {
    throw std::runtime_error("Codec " + decoder_name + " not found.");
  }

  ctx_ = avcodec_alloc_context3(codec_);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context");
  }
}

decode::decode(const enum AVCodecID id, const bool using_hardware) noexcept : decode() {
  if (using_hardware) {
    if (id != AV_CODEC_ID_H264 && id != AV_CODEC_ID_HEVC) {
      throw std::runtime_error("Unsupported hard decoding format: " +
                               std::string(avcodec_get_name(id) ? avcodec_get_name(id) : "Unknown codec"));
    }
    const std::string hard_decoder_name = id == AV_CODEC_ID_H264 ? "h264_cuvid" : "hevc_cuvid";
    //
    codec_ = avcodec_find_decoder_by_name(hard_decoder_name.c_str());
    if (!codec_) {
      throw std::runtime_error("Codec " + hard_decoder_name + " not found.");
    }
  } else {
    codec_ = avcodec_find_decoder(id);
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

decode::~decode() {
  avcodec_free_context(&ctx_);
  ctx_   = nullptr;
  codec_ = nullptr;
}

void decode::set_parameters(const AVCodecContext* params) noexcept {
  // 参考 ffmpeg/libavcodec/utils.c 的函数 avcodec_parameters_to_context 的实现
  ctx_->codec_type = params->codec_type;
  ctx_->codec_id   = params->codec_id;
  ctx_->codec_tag  = params->codec_tag;

  ctx_->bit_rate              = params->bit_rate;
  ctx_->bits_per_coded_sample = params->bits_per_coded_sample;
  ctx_->bits_per_raw_sample   = params->bits_per_raw_sample;
  ctx_->profile               = params->profile;
  ctx_->level                 = params->level;

  switch (params->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      ctx_->pix_fmt                = params->pix_fmt;
      ctx_->width                  = params->width;
      ctx_->height                 = params->height;
      ctx_->field_order            = params->field_order;
      ctx_->color_range            = params->color_range;
      ctx_->color_primaries        = params->color_primaries;
      ctx_->color_trc              = params->color_trc;
      ctx_->colorspace             = params->colorspace;
      ctx_->chroma_sample_location = params->chroma_sample_location;
      ctx_->sample_aspect_ratio    = params->sample_aspect_ratio;
      ctx_->has_b_frames           = params->has_b_frames;
      break;
    case AVMEDIA_TYPE_AUDIO:
      ctx_->sample_fmt     = params->sample_fmt;
      ctx_->channel_layout = params->channel_layout;
      ctx_->channels       = params->channels;
      ctx_->sample_rate    = params->sample_rate;
      ctx_->block_align    = params->block_align;
      ctx_->frame_size     = params->frame_size;
      ctx_->delay = ctx_->initial_padding = params->initial_padding;
      ctx_->trailing_padding              = params->trailing_padding;
      ctx_->seek_preroll                  = params->seek_preroll;
      break;
    case AVMEDIA_TYPE_SUBTITLE:
      ctx_->width  = params->width;
      ctx_->height = params->height;
      break;
    default:
      break;
  }

  // extradata
  if (params->extradata) {
    av_freep(&ctx_->extradata);
    ctx_->extradata = static_cast<uint8_t*>(av_mallocz(params->extradata_size));
    if (!ctx_->extradata) {
      throw std::runtime_error("Failed to allocate memory for extradata.");
    }
    memcpy(ctx_->extradata, params->extradata, params->extradata_size);
    ctx_->extradata_size = params->extradata_size;
  }
}

void decode::create_decoder(const AVCodecContext* params) {
  this->set_parameters(params);

  const auto ret = avcodec_open2(ctx_, codec_, nullptr);
  if (ret < 0) {
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    const std::string warn_msg =
        "Could not open codec: " + std::string(av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret));
    throw std::runtime_error(warn_msg);
  }
}

auto decode::decoding(av_packet pkt)const -> std::vector<av_frame> {
  std::vector<av_frame> frame_list;

  auto ret = avcodec_send_packet(ctx_, pkt.get());
  if (ret < 0) {
    throw std::runtime_error("Error sending a packet for decoding: " + std::to_string(ret));
  }

  while (ret >= 0) {
    av_frame frame;  // 由于解码后的 frame 内存占用较大, 所以考虑移动来处理, 又由于移动后需要重新申请内存,
                     // 所以将其声明移动到循环中
    ret = avcodec_receive_frame(ctx_, frame.get());
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return frame_list;
    } else if (ret < 0) {
      throw std::runtime_error("Error during decoding: " + std::to_string(ret));
    }

    frame_list.emplace_back(std::move(frame));  // 移动构造, 节省内存开销
  }

  return frame_list;
}
