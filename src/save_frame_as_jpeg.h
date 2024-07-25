#ifndef SAVE_FRAME_AS_JPEG_H
#define SAVE_FRAME_AS_JPEG_H
#include <fstream>
#include <gsl/util>
#include <iostream>
#include <ostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

static void save_frame_as_jpeg(AVFrame* frame, int width, int height, const std::string& filename) {
  AVPacket* pkt    = av_packet_alloc();
  auto pkg_release = gsl::finally([&]() { av_packet_free(&pkt); });

  if (!pkt) {
    std::cerr << "Error allocating packet" << std::endl;
    return;
  }
  AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!codec) {
    std::cerr << "Codec not found" << std::endl;
    return;
  }
  AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  auto codec_ctx_release    = gsl::finally([&]() { avcodec_free_context(&codec_ctx); });
  if (!codec_ctx) {
    std::cerr << "Could not allocate video codec context" << std::endl;
    return;
  }
  codec_ctx->bit_rate     = 400000;
  codec_ctx->width        = width;
  codec_ctx->height       = height;
  codec_ctx->time_base    = (AVRational){1, 25};
  codec_ctx->framerate    = (AVRational){25, 1};
  codec_ctx->gop_size     = 10;
  codec_ctx->max_b_frames = 0;
  codec_ctx->pix_fmt      = AV_PIX_FMT_YUVJ420P;

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    return;
  }

  int ret = avcodec_send_frame(codec_ctx, frame);
  if (ret < 0) {
    std::cerr << "Error sending a frame for encoding" << std::endl;
    return;
  }

  while (ret >= 0) {
    ret = avcodec_receive_packet(codec_ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0) {
      std::cerr << "Error during encoding" << std::endl;
      return;
    }

    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write((char*)pkt->data, pkt->size);
    file.close();

    av_packet_unref(pkt);
  }
}

#endif  // SAVE_FRAME_AS_JPEG_H
