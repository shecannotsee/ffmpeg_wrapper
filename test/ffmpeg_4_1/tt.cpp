#include <she_test.h>

#include "test_support.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
}

#include <fstream>
#include <iostream>
#include <sstream>

void savePacketData(const AVPacket* pkt, std::ofstream& outFile) {
  outFile.write((const char*)pkt->data, pkt->size);
}

SHE_TEST(tt, tt) {
  const char* rtspUrl = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./tt";
  test_support::create_dir(output_dir);
  int gopIndex        = 0;
  std::ofstream outFile;

  av_register_all();
  avformat_network_init();

  AVFormatContext* formatCtx = nullptr;
  if (avformat_open_input(&formatCtx, rtspUrl, nullptr, nullptr) != 0) {
    std::cerr << "Could not open input stream." << std::endl;
    return -1;
  }

  if (avformat_find_stream_info(formatCtx, nullptr) < 0) {
    std::cerr << "Could not find stream information." << std::endl;
    avformat_close_input(&formatCtx);
    return -1;
  }

  av_dump_format(formatCtx, 0, rtspUrl, 0);

  int videoStreamIndex = -1;
  for (unsigned int i = 0; i < formatCtx->nb_streams; ++i) {
    if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      break;
    }
  }

  if (videoStreamIndex == -1) {
    std::cerr << "Could not find video stream." << std::endl;
    avformat_close_input(&formatCtx);
    return -1;
  }

  AVCodecParameters* codecPar = formatCtx->streams[videoStreamIndex]->codecpar;
  AVCodec* codec              = avcodec_find_decoder(codecPar->codec_id);
  if (!codec) {
    std::cerr << "Unsupported codec!" << std::endl;
    avformat_close_input(&formatCtx);
    return -1;
  }

  AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
  if (!codecCtx) {
    std::cerr << "Could not allocate video codec context." << std::endl;
    avformat_close_input(&formatCtx);
    return -1;
  }

  if (avcodec_parameters_to_context(codecCtx, codecPar) < 0) {
    std::cerr << "Could not copy codec context." << std::endl;
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    return -1;
  }

  if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
    std::cerr << "Could not open codec." << std::endl;
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    return -1;
  }

  AVPacket packet;
  av_init_packet(&packet);

  while (av_read_frame(formatCtx, &packet) >= 0) {
    if (packet.stream_index == videoStreamIndex) {
      if (packet.flags & AV_PKT_FLAG_KEY) {
        // Close previous file if open
        if (outFile.is_open()) {
          outFile.close();
        }

        // Create a new file for the new GOP
        std::ostringstream filename;
        filename << output_dir+"/output_gop_" << gopIndex++ << ".h264";
        outFile.open(filename.str(), std::ios::out | std::ios::binary);

        if (!outFile.is_open()) {
          std::cerr << "Could not open output file: " << filename.str() << std::endl;
          av_packet_unref(&packet);
          break;
        }
      }

      if (outFile.is_open()) {
        savePacketData(&packet, outFile);
      }
    }
    av_packet_unref(&packet);
  }

  // Close last file
  if (outFile.is_open()) {
    outFile.close();
  }

  avcodec_free_context(&codecCtx);
  avformat_close_input(&formatCtx);
  avformat_network_deinit();

  return true;
}
