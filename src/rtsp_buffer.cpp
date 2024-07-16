#include "rtsp_buffer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

rtsp_buffer::rtsp_buffer() {
  avformat_network_init();
}

rtsp_buffer::~rtsp_buffer() {
  stop_buffering();
}

void rtsp_buffer::start_buffering(std::string rtsp_url) {
  if (is_buffering_) {
    throw std::runtime_error("Buffering is already started.");
  }
  is_buffering_     = true;
  buffering_thread_ = std::thread(&rtsp_buffer::buffer_rtsp_stream, this, rtsp_url);
}

void rtsp_buffer::stop_buffering() {
  if (!is_buffering_) {
    return;
  }

  is_buffering_ = false;
  if (buffering_thread_.joinable()) {
    buffering_thread_.join();
  }
}

void rtsp_buffer::buffer_rtsp_stream(std::string rtsp_url) {
  AVFormatContext* fmt_ctx = nullptr;

  if (avformat_open_input(&fmt_ctx, rtsp_url.c_str(), nullptr, nullptr) != 0) {
    throw std::runtime_error("Failed to open input stream.");
  }

  if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
    avformat_close_input(&fmt_ctx);
    throw std::runtime_error("Failed to find stream info.");
  }

  int video_stream_index = -1;
  for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
    if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }

  if (video_stream_index == -1) {
    avformat_close_input(&fmt_ctx);
    throw std::runtime_error("Failed to find video stream.");
  }

  AVPacket packet;
  while (is_buffering_) {
    // 获取帧失败: 可能意味着流结束或者其他异常情况
    if (av_read_frame(fmt_ctx, &packet) < 0) {
      break;
    }
    // 非视频帧信息
    if (packet.stream_index != video_stream_index) {
      av_packet_unref(&packet);
      continue;
    }

    // 存数据
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    // 当此时帧为关键帧的时候,需要存储额外的信息:包括SPS和PPS数据
    if (packet.flags & AV_PKT_FLAG_KEY) {
      gop_segments_.emplace_back();  // 只在关键帧的时候添加一个gop组
      gop_segments_.back().earliest_pts = packet.pts;
      // 添加额外信息
      AVCodecParameters* codecpar    = fmt_ctx->streams[video_stream_index]->codecpar;
      gop_segments_.back().extradata = bytes(codecpar->extradata, codecpar->extradata + codecpar->extradata_size);
    }

    if (!gop_segments_.empty()) {
      // 构造一个帧信息
      frame_info temp;
      temp.pkg_data = bytes(packet.data, packet.data + packet.size);
      temp.pts      = packet.pts;
      // 将一个帧信息添加到gop中
      gop_segments_.back().frame.emplace_back(temp);
    }

    av_packet_unref(&packet);
  }

  avformat_close_input(&fmt_ctx);
}
