#include <she_test.h>

#include <gsl/util>
#include <thread>

#include "api.h"
#include "rtsp_buffer.h"
#include "test_support.h"

SHE_TEST(api_mock, sample) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./api_test/";
  test_support::create_dir(output_dir);

  std::atomic<bool> data_ready{false};
  std::vector<save_evidence_info> pass_draw_box;

  std::thread py_mock([&]() {
    rtsp_buffer stream;
    stream.start_buffering(rtsp_url);
    std::this_thread::sleep_for(std::chrono::seconds(10));
    stream.stop_buffering();

    auto buffer = stream.get_buffer();

    // 从buffer_1中获取第三组的所有frame的pts，试图在buffer_2中定位并进行绘制
    pre_decoding rtsp_stream;
    rtsp_stream.set_format_from(rtsp_url);
    decoding<AVCodecID> decoder(rtsp_stream.get_video_type());

    int num                  = 0;
    constexpr int num_target = 3;
    for (auto gop : buffer) {
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
      if (num == num_target) {
        // 送入解码器
        auto frames         = decoder.independent_decoder(gop.extradata, packets);
        auto frames_release = gsl::finally([&]() {
          for (int i = 0; i < frames.size(); i++) {
            av_frame_free(&frames[i]);
          }
        });
        for (auto frame : frames) {
          pass_draw_box.emplace_back();
          pass_draw_box.back().pts = frame->pts;
          pass_draw_box.back().boxs = get_detections_test();
        }

        break;
      }
    }

    data_ready = true;
  });

  std::thread api_th([&]() {
    api cache_module;
    cache_module.start_cache(rtsp_url, 60);

    while (!data_ready) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    cache_module.save_evidence(pass_draw_box, output_dir + "evidence.h264");
  });

  py_mock.join();
  api_th.join();
  return true;
}