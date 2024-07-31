#include <decode_support.h>
#include <she_log.h>
#include <she_test.h>

#include <gsl/util>

#include "draw_on_the_picture.h"
#include "rtsp_buffer.h"
#include "save_frame_as_jpeg.h"
#include "test_support.h"

SHE_TEST(draw_on_the_picture, detection_boxes) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./draw_on_the_picture/";
  test_support::create_dir(output_dir);

  // 独立解码器设置
  pre_decoding rtsp_stream;
  rtsp_stream.set_format_from(rtsp_url);
  decoding<AVCodecID> decoder(rtsp_stream.get_video_type());

  // 获取缓存,时间为10s
  rtsp_buffer stream;
  stream.start_buffering(rtsp_url);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  stream.stop_buffering();
  auto bufffer = stream.get_buffer();

  int frame_num = 0;
  // 使用解码器进行解码
  for (auto gop : bufffer) {
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
    std::vector<AVFrame*> frames = decoder.independent_decoder(gop.extradata, packets);
    auto frames_release          = gsl::finally([&]() {
      for (int i = 0; i < frames.size(); i++) {
        av_frame_free(&frames[i]);
      }
    });

    for (const auto frame : frames) {
      // 画框
      draw_detection_boxes(frame, get_detections_test());
      // 存为jpeg
      save_frame_as_jpeg(frame, frame->width, frame->height, output_dir + std::to_string(++frame_num) + ".jpg");
    }
  }

  return true;
}
