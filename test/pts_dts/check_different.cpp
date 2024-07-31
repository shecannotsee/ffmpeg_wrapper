#include <she_test.h>

#include <gsl/util>

#include "decode_support.h"
#include "rtsp_buffer.h"
#include "save_frame_as_jpeg.h"
#include "test_support.h"

SHE_TEST(pts_dts, check_different) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./pts_dts/";
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
    // 送入解码器
    auto frames         = decoder.independent_decoder(gop.extradata, packets);
    auto frames_release = gsl::finally([&]() {
      for (int i = 0; i < frames.size(); i++) {
        av_frame_free(&frames[i]);
      }
    });

    std::cout << "==\n";
    for (int i = 0; i < frames.size(); i++) {
      std::cout << "pkg-dts:" << packets[i]->dts << ";";
      std::cout << "pkg-pts:" << packets[i]->pts << ";";
      std::cout << "frm-pts:" << frames[i]->pts << ";\n";
      using std::to_string;
      static int count = 0;
      count++;
      std::string count_str = to_string(count);
      if (count < 10) {
        count_str = "0" + count_str;
      }
      const std::string id = count_str + "pkgdts" + to_string(packets[i]->dts) + "_" + "pkgpts" +
                             to_string(packets[i]->pts) + "_" + "frmpts" + to_string(frames[i]->pts);
      const std::string file_name = output_dir + id + ".jpg";
      save_frame_as_jpeg(frames[i], frames[i]->width, frames[i]->height, file_name);
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  return true;
}
