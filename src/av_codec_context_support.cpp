#include "av_codec_context_support.h"

#include <stdexcept>

av_codec_context_support::av_codec_context_support() noexcept : ctx_(nullptr) {
  alloc_default();
}

void av_codec_context_support::alloc_default() noexcept {
  ctx_ = avcodec_alloc_context3(nullptr);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context");
  }
}

void av_codec_context_support::alloc_default(enum AVCodecID id) noexcept {
  const auto codec = avcodec_find_encoder(id);
  ctx_             = avcodec_alloc_context3(codec);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context with " +
                             std::string(avcodec_get_name(id) ? avcodec_get_name(id) : "Unknown codec"));
  }
}

void av_codec_context_support::alloc_default(const std::string& codec_name) noexcept {
  auto codec = avcodec_find_encoder_by_name(codec_name.c_str());
  ctx_       = avcodec_alloc_context3(codec);
  if (!ctx_) {
    throw std::runtime_error("Could not allocate video codec context with " + codec_name);
  }
}

void av_codec_context_support::release() noexcept {
  if (ctx_ != nullptr) {
    avcodec_free_context(&ctx_);
    ctx_ = nullptr;
  }
}

av_codec_context_support::~av_codec_context_support() noexcept {
  release();
}

auto av_codec_context_support::get_jpeg_encode(int width, int height) noexcept -> const AVCodecContext* {
  release();
  alloc_default(AV_CODEC_ID_MJPEG);
  ctx_->bit_rate     = 400000;
  ctx_->codec_id     = AV_CODEC_ID_MJPEG;
  ctx_->codec_type   = AVMEDIA_TYPE_VIDEO;
  ctx_->width        = width;
  ctx_->height       = height;
  ctx_->time_base    = (AVRational){1, 25};
  ctx_->framerate    = (AVRational){25, 1};
  ctx_->gop_size     = 10;
  ctx_->max_b_frames = 0;
  ctx_->pix_fmt      = AV_PIX_FMT_YUVJ420P;
  return ctx_;
}

auto av_codec_context_support::get_h264_nvenc_encode(const demux& stream) noexcept -> const AVCodecContext* {
  auto param = stream.get_codec_parameters<demux::type::video>();
  release();
  alloc_default("h264_nvenc");
  ctx_->bit_rate     = param->bit_rate;
  ctx_->codec_id     = param->codec_id;
  ctx_->codec_type   = param->codec_type;
  ctx_->width        = param->width;
  ctx_->height       = param->height;
  ctx_->time_base    = (AVRational){1, 25};
  ctx_->framerate    = (AVRational){25, 1};
  ctx_->gop_size     = 10;
  ctx_->max_b_frames = 0;
  ctx_->pix_fmt      = AV_PIX_FMT_NV12;
  return ctx_;
}

auto av_codec_context_support::get_h264_encode(const demux& stream) noexcept -> const AVCodecContext* {
  auto param = stream.get_codec_parameters<demux::type::video>();
  release();
  alloc_default(AV_CODEC_ID_H264);
  ctx_->bit_rate     = param->bit_rate;
  ctx_->codec_id     = param->codec_id;
  ctx_->codec_type   = param->codec_type;
  ctx_->width        = param->width;
  ctx_->height       = param->height;
  ctx_->time_base    = (AVRational){1, 25};
  ctx_->framerate    = (AVRational){25, 1};
  ctx_->gop_size     = 10;
  ctx_->max_b_frames = 0;
  ctx_->pix_fmt      = AV_PIX_FMT_YUVJ420P;
  return ctx_;
}
