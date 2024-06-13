#include <she_test.h>

#include <gsl/gsl>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <test_support.h>

#include <deque>
#include <iostream>

namespace {
void save_frame_as_jpeg(AVFrame* frame, int width, int height, const std::string& filename) {
  int ret;
  AVPacket* pkt = av_packet_alloc();
  if (!pkt) {
    std::cerr << "Error allocating packet" << std::endl;
    return;
  }
  // 查找JPEG编码器
  AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!codec) {
    std::cerr << "Codec not found" << std::endl;
    return;
  }

  AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  auto codec_ctx_release    = gsl::finally([&]() { avcodec_free_context(&codec_ctx); });
  /* check */ {
    if (!codec_ctx) {
      std::cerr << "Could not allocate video codec context" << std::endl;
      return;
    }
  }

  // 设置编码器参数
  codec_ctx->bit_rate     = 400000;
  codec_ctx->width        = width;
  codec_ctx->height       = height;
  codec_ctx->time_base    = (AVRational){1, 25};
  codec_ctx->framerate    = (AVRational){25, 1};
  codec_ctx->gop_size     = 10;
  codec_ctx->max_b_frames = 0;  // 不使用B帧
  codec_ctx->pix_fmt      = AV_PIX_FMT_YUVJ420P;

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    std::cerr << "Could not open codec" << std::endl;
    return;
  }

  ret = avcodec_send_frame(codec_ctx, frame);
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

  av_packet_free(&pkt);
}

}  // namespace

SHE_TEST(crop_video_stream, method_1) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./crop_video_stream";
  test_support::create_dir(output_dir);

  // 打开rtsp流
  AVFormatContext* format_ctx = avformat_alloc_context();  ///< 流上下文
  auto format_ctx_release     = gsl::finally([&]() { avformat_close_input(&format_ctx); });
  // Open RTSP stream
  if (avformat_open_input(&format_ctx, rtsp_url.c_str(), NULL, NULL) != 0) {
    fprintf(stderr, "Could not open input file.\n");
    return false;
  }
  // 检索流信息
  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information.\n");
    return false;
  }
  // dump
  av_dump_format(format_ctx, 0, rtsp_url.c_str(), 0);
  // 从第一帧数获取流信息
  AVCodec* codec                      = nullptr;
  AVCodecParameters* codec_parameters = nullptr;
  int video_stream_index              = -1;
  // Find the first video stream
  for (int i = 0; i < format_ctx->nb_streams; i++) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      codec_parameters   = format_ctx->streams[i]->codecpar;
      codec              = avcodec_find_decoder(codec_parameters->codec_id);
      if (!codec) {
        std::cerr << "Unsupported codec!" << std::endl;
        return false;
      }
      break;
    }
  }

  /* 设置解码器 */
  // Create codec context
  AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    std::cerr << "Failed to allocate codec context" << std::endl;
    return -1;
  }

  if (avcodec_parameters_to_context(codec_ctx, codec_parameters) < 0) {
    std::cerr << "Failed to copy codec parameters to codec context" << std::endl;
    return -1;
  }

  // Open codec
  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    std::cerr << "Failed to open codec" << std::endl;
    return -1;
  }

  /* 准备按帧处理了 */
  // 准备按帧处理了
  AVPacket* packet = av_packet_alloc();
  if (!packet) {
    std::cerr << "Failed to allocate AVPacket" << std::endl;
    return false;
  }

  AVFrame* frame = av_frame_alloc();
  if (!frame) {
    std::cerr << "Failed to allocate AVFrame" << std::endl;
    return false;
  }

  int frame_count = 0;

  // Read frames
  while (av_read_frame(format_ctx, packet) >= 0) {
    if (packet->stream_index == video_stream_index) {
      int response = avcodec_send_packet(codec_ctx, packet);
      if (response < 0) {
        std::cerr << "Error while sending a packet to the decoder: " << response << std::endl;
        break;
      }

      while (response >= 0) {
        response = avcodec_receive_frame(codec_ctx, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
          break;
        else if (response < 0) {
          std::cerr << "Error while receiving a frame from the decoder: " << response << std::endl;
          break;
        }

        // Save frame to file (for example, as JPEG)
        std::string filename = std::string(output_dir) + "/frame" + std::to_string(frame_count) + ".jpg";
        save_frame_as_jpeg(frame, codec_ctx->width, codec_ctx->height, filename);

        frame_count++;
      }
    }

    av_packet_unref(packet);
  }

  return true;
}