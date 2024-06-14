#include <she_test.h>

#include <gsl/gsl>
#include <thread>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <frame_info.h>
#include <packet_info.h>
#include <test_support.h>

#include <deque>
#include <fstream>
#include <iostream>
#include <vector>

namespace {
void save_frame_as_jpeg(AVFrame* frame, int width, int height, const std::string& filename) {
  int ret;
  AVPacket* pkt = av_packet_alloc();
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

  AVFormatContext* format_ctx = avformat_alloc_context();
  auto format_ctx_release     = gsl::finally([&]() { avformat_close_input(&format_ctx); });

  if (avformat_open_input(&format_ctx, rtsp_url.c_str(), NULL, NULL) != 0) {
    fprintf(stderr, "Could not open input file.\n");
    return false;
  }

  if (avformat_find_stream_info(format_ctx, NULL) < 0) {
    fprintf(stderr, "Could not find stream information.\n");
    return false;
  }

  av_dump_format(format_ctx, 0, rtsp_url.c_str(), 0);

  // 获取视频流信息
  int video_stream_index = -1;
  for (unsigned int i = 0; i < format_ctx->nb_streams; ++i) {
    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }
  /* check */ {
    if (video_stream_index == -1) {
      std::cerr << "Could not find video stream." << std::endl;
      return false;
    }
  }

  // 配置编码器
  AVCodecParameters* codec_parameters = format_ctx->streams[video_stream_index]->codecpar;
  AVCodec* codec                      = avcodec_find_decoder(codec_parameters->codec_id);
  /* check */ {
    if (!codec) {
      std::cerr << "Unsupported codec!" << std::endl;
      avformat_close_input(&format_ctx);
      return false;
    }
  }

  AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
  auto codec_ctx_release    = gsl::finally([&]() { avcodec_free_context(&codec_ctx); });
  /* check */ {
    if (!codec_ctx) {
      std::cerr << "Failed to allocate codec context" << std::endl;
      return false;
    }
  }

  if (avcodec_parameters_to_context(codec_ctx, codec_parameters) < 0) {
    std::cerr << "Failed to copy codec parameters to codec context" << std::endl;
    return false;
  }

  if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
    std::cerr << "Failed to open codec" << std::endl;
    return false;
  }

  /* 准备工作完毕 */
  // 为编解码做准备了
  AVPacket* packet    = av_packet_alloc();
  auto packet_release = gsl::finally([&]() { av_packet_free(&packet); });
  /* check */ {
    if (!packet) {
      std::cerr << "Failed to allocate AVPacket" << std::endl;
      return false;
    }
  }

  AVFrame* frame     = av_frame_alloc();
  auto frame_release = gsl::finally([&]() { av_frame_free(&frame); });
  /* check */ {
    if (!frame) {
      std::cerr << "Failed to allocate AVFrame" << std::endl;
      return false;
    }
  }

  // 开始编解码了
  using packet_data = std::vector<uint8_t>;
  std::vector<std::vector<packet_data>> GOP_lists{1};  // group of picture
  int gop_lists_index = 0;

  int video_packet_count = 0;
  int frame_count        = 0;

  while (av_read_frame(format_ctx, packet) >= 0) {
    // 处理视频流
    if (packet->stream_index == video_stream_index) {
      /* 1.首先缓存数据 */ {
        // 是关键帧
        if (packet->flags & AV_PKT_FLAG_KEY) {
          GOP_lists.emplace_back();
          gop_lists_index++;
          // 只有关键帧才添加SPS 和 PPS 数据
          GOP_lists[gop_lists_index].emplace_back(codec_ctx->extradata,
                                                  codec_ctx->extradata + codec_ctx->extradata_size);
        }
        // 添加当前包到当前 GOP
        GOP_lists[gop_lists_index].emplace_back(packet->data, packet->data + packet->size);
        std::cout << "pts:" << packet->pts << ",dts:" << packet->dts << std::endl;
      }
      /* 2.做解码 */
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
        // std::cout << "pts:" << packet->pts << ",dts:" << packet->dts << std::endl;
        // Save frame to file (for example, as JPEG)
        // std::string filename = std::string(output_dir) + "/frame" + std::to_string(++frame_count) + ".jpg";
        // save_frame_as_jpeg(frame, codec_ctx->width, codec_ctx->height, filename);
      }
    }

    av_packet_unref(packet);
  }

  std::cout << "decode done!"<< std::endl;
  // TODO:只存储了数据，没有存储pts和dts，为了兼容性需要用结构体包这俩数据
  for (int gp_index = 1; gp_index < GOP_lists.size(); gp_index++) {
    for (int pkg_index = 0; pkg_index < GOP_lists[gp_index].size(); pkg_index++) {
      AVPacket* pkg = reinterpret_cast<AVPacket*>(GOP_lists[gp_index][pkg_index].data());
      std::cout << "pts:" << pkg->pts << ",dts:" << pkg->dts << std::endl;

    }
  }
  std::cout << "test done!start write"<< std::endl;


  auto writeGopListToFile = [](const std::vector<packet_data>& gop_list, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile.is_open()) {
      std::cerr << "Failed to open file for writing: " << filename << std::endl;
      return;
    }

    for (const auto& packet : gop_list) {
      uint32_t size = packet.size();
      outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));
      outfile.write(reinterpret_cast<const char*>(packet.data()), size);
    }

    outfile.close();
  };

  for (size_t i = 0; i < GOP_lists.size(); ++i) {
    std::string filename = output_dir + "/gop_list_" + std::to_string(i+1) + ".h264";
    writeGopListToFile(GOP_lists[i], filename);
  }

  return true;
}
