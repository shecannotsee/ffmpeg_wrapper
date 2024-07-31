#include <she_test.h>

#include <gsl/util>

#include "decode_support.h"
#include "draw_on_the_picture.h"
#include "rtsp_buffer.h"
#include "save_frame_as_jpeg.h"
#include "test_support.h"

SHE_TEST(frame_align, error_with_flows) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./frame_align/";
  test_support::create_dir(output_dir);

  // 独立解码器设置
  pre_decoding rtsp_stream;
  rtsp_stream.set_format_from(rtsp_url);
  decoding<AVCodecID> decoder(rtsp_stream.get_video_type());

  // 获取缓存,时间为10s
  rtsp_buffer stream_1, stream_2;
  stream_1.start_buffering(rtsp_url);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  stream_2.start_buffering(rtsp_url);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  stream_1.stop_buffering();
  stream_2.stop_buffering();
  auto bufffer_1 = stream_1.get_buffer();
  auto bufffer_2 = stream_2.get_buffer();

  struct tag {
    std::vector<int> pts;
    std::vector<std::vector<detection>> boxs;
  };
  tag draw_target;
  // 从buffer_1中获取第三组的所有frame的pts，试图在buffer_2中定位并进行绘制
  int num = 0;
  for (auto gop : bufffer_1) {
    num++;
    // 组装 std::vector<AVPacket*>
    std::vector<AVPacket*> packets;
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    for (const auto& packet_info : gop.packet) {
      AVPacket* temp;
      temp       = av_packet_alloc();
      temp->data = static_cast<uint8_t*>(av_malloc(packet_info.pkg_data.size()));

      memcpy(temp->data, packet_info.pkg_data.data(), packet_info.pkg_data.size());
      temp->size = static_cast<int>(packet_info.pkg_data.size());
      temp->pts  = packet_info.pts;
      temp->dts  = packet_info.dts;
      packets.emplace_back(temp);
    }
    if (num == 3) {
      // 送入解码器
      auto frames         = decoder.independent_decoder(gop.extradata, packets);
      auto frames_release = gsl::finally([&]() {
        for (int i = 0; i < frames.size(); i++) {
          av_frame_free(&frames[i]);
        }
      });
      for (auto frame : frames) {
        draw_target.pts.emplace_back(frame->pts);
        draw_target.boxs.emplace_back(get_detections_test());
      }

      break;
    }
  }

  // 在buffer_2中查找并绘制
  auto first_pts = draw_target.pts;
  for (auto gop : bufffer_2) {
    // 组装 std::vector<AVPacket*>
    std::vector<AVPacket*> packets;
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    for (const auto& packet_info : gop.packet) {
      AVPacket* temp;
      temp       = av_packet_alloc();
      temp->data = static_cast<uint8_t*>(av_malloc(packet_info.pkg_data.size()));

      memcpy(temp->data, packet_info.pkg_data.data(), packet_info.pkg_data.size());
      temp->size = static_cast<int>(packet_info.pkg_data.size());
      temp->pts  = packet_info.pts;
      temp->dts  = packet_info.dts;
      packets.emplace_back(temp);
    }
    // 开始校准
    int offset = (packets[0]->dts % 100) - (first_pts % 100);
    for (auto packet : packets) {
    }
  }

  return true;
}
