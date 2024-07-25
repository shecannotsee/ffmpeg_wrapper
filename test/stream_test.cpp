#include <she_test.h>

#include <gsl/util>

#include "rtsp_buffer.h"
#include "test_support.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

#include <md5.h>
#include <netinet/in.h>
#include <she_log.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>



void f(std::string rtsp_url, std::string thread_id) {
  she_log::logger pen;
  {
    const auto local_file = std::make_shared<she_log::local_file>("./test_log" + thread_id + ".txt");
    local_file->set_format(she_log::log_format::SHE_LOG);
    local_file->set_level(she_log::log_level::ALL);
    pen.add_output_method(local_file);
    const auto console = std::make_shared<she_log::console>();
    console->set_format(she_log::log_format::SHE_LOG);
    console->set_level(she_log::log_level::ALL);
    pen.add_output_method(console);
  }

  AVFormatContext *fmt_ctx = nullptr;
  auto fmt_ctx_release     = gsl::finally([&]() { avformat_close_input(&fmt_ctx); });

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


  AVStream *stream     = fmt_ctx->streams[video_stream_index];
  int64_t start_time   = fmt_ctx->start_time;  // 首帧的传输的相对时间
  AVRational time_base = stream->time_base;    // time_base.den: It means how many units there are in 1 second

  AVPacket packet;
  auto start_timestamp = std::chrono::system_clock::now();
  while (true) {
    // 获取帧失败: 可能意味着流结束或者其他异常情况
    if (av_read_frame(fmt_ctx, &packet) < 0) {
      break;
    }

    // 非视频帧信息
    if (packet.stream_index != video_stream_index) {
      av_packet_unref(&packet);
      continue;
    }

    // [==处理数据==]: 通过start_timestamp,start_time,time_base.den和pts计算出当前帧的绝对时间以毫秒进行展示
    std::string packet_data_str(reinterpret_cast<char *>(packet.data), packet.size);

    int64_t dts_ms = (packet.dts - start_time) * av_q2d(time_base) * 1000;
    // 计算绝对时间点
    auto absolute_time = start_timestamp + std::chrono::milliseconds(dts_ms);
    // 将绝对时间点转换为tm结构体以便于格式化输出
    std::time_t display_time_t = std::chrono::system_clock::to_time_t(absolute_time);
    std::tm display_tm         = *std::localtime(&display_time_t);
    auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(absolute_time.time_since_epoch()).count() % 1000;
    // 输出当前帧的信息，包括时分秒和毫秒
    pen.record<she_log::log_level::DEBUG>(she_log::fmt("Frame dts::{} - Absolute Time: {}{}:{}", packet.dts,
                                                       std::put_time(&display_tm, "%F %T."), milliseconds,
                                                       MD5(packet_data_str).toStr()));
    // [==处理结束==]

    av_packet_unref(&packet);
  }
}

SHE_TEST(test, stream_test) {
  std::string rtsp_url = "rtsp://localhost:8554/mystream";

  std::thread t1(f,rtsp_url,"1");
  std::this_thread::sleep_for(std::chrono::seconds(3));
  std::thread t2(f,rtsp_url,"2");

  t1.join();
  t2.join();

  return true;
}
