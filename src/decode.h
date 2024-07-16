#ifndef DECODE_H
#define DECODE_H

#include <iostream>

#include "gop_segment.h"
#include "save_frame_as_jpeg.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <stdexcept>

void decode_gop(const gop_segment& segment, const std::string& output_folder) {
  AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec) {
    throw std::runtime_error("Failed to find H.264 codec.");
  }

  AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  auto codec_ctx_release    = gsl::finally([&]() { avcodec_free_context(&codec_ctx); });
  if (!codec_ctx) {
    throw std::runtime_error("Failed to allocate codec context.");
  }

  // 设置extradata (SPS和PPS)
  if (!segment.extradata.empty()) {
    codec_ctx->extradata = static_cast<uint8_t*>(av_malloc(segment.extradata.size()));
    if (!codec_ctx->extradata) {
      avcodec_free_context(&codec_ctx);
      throw std::runtime_error("Failed to allocate memory for extradata.");
    }
    memcpy(codec_ctx->extradata, segment.extradata.data(), segment.extradata.size());
    codec_ctx->extradata_size = segment.extradata.size();
  }

  if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
    avcodec_free_context(&codec_ctx);
    throw std::runtime_error("Failed to open codec.");
  }

  AVFrame* frame     = av_frame_alloc();
  auto frame_release = gsl::finally([&]() { av_frame_free(&frame); });

  static int count = 0;
  count++;
  for (const auto& packet_info : segment.packet) {
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = static_cast<uint8_t*>(av_malloc(packet_info.pkg_data.size()));
    if (!packet.data) {
      av_frame_free(&frame);
      avcodec_free_context(&codec_ctx);
      throw std::runtime_error("Failed to allocate memory for packet data.");
    }
    memcpy(packet.data, packet_info.pkg_data.data(), packet_info.pkg_data.size());
    packet.size = packet_info.pkg_data.size();
    packet.pts  = packet_info.pts;
    packet.dts  = packet_info.dts;

    int response = avcodec_send_packet(codec_ctx, &packet);
    if (response < 0) {
      std::cerr << "Error while sending a packet to the decoder: " << response << std::endl;
      break;
    }
    while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
      // 保存解码后的帧, 例如保存为图片
      std::string file_name = std::to_string(frame->pts) + ".jpg";
      save_frame_as_jpeg(frame, codec_ctx->width, codec_ctx->height,
                         output_folder + "/gop_" + std::to_string(count) + "_" + file_name);
    }
  }
}


#endif  // DECODE_H
