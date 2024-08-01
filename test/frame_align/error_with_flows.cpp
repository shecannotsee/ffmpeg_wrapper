#include <she_test.h>

#include <gsl/util>

#include "decode_support.h"
#include "draw_on_the_picture.h"
#include "encode_support.h"
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
  int num                  = 0;
  constexpr int num_target = 3;
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
    if (num == num_target) {
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
  // 首先获取偏移量
  int offset = 0;
  {
    // 获取要绘制的
    auto first_pts                  = draw_target.pts[0];
    int first_bigger_than_first_pts = 0;
    for (auto gop : bufffer_2) {
      for (auto packet_info : gop.packet) {
        if (packet_info.dts == first_pts) {
          offset = 0;
          break;
        } else if (packet_info.dts > first_pts) {  // 找到了第一个大于画框组的帧的pts就可以开始计算offset了
          first_bigger_than_first_pts = packet_info.dts;
          offset                      = first_bigger_than_first_pts - first_pts;
          break;
        }
      }
      if (offset != 0) {
        break;
      }
    }
  }
  // 获取需要解码和绘制的组
  int start_gop_num = 0;
  int end_top_num   = 0;
  {
    int draw_start_pts = draw_target.pts[0] + offset;
    int draw_end_pts   = draw_target.pts.back() + offset;
    for (int i = 0; i < bufffer_2.size(); ++i) {
      for (auto pkg : bufffer_2[i].packet) {
        if (draw_start_pts == pkg.dts) {
          start_gop_num = i;
        }
        if (draw_end_pts == pkg.dts) {
          end_top_num = i;
          break;
        }
      }
      if (end_top_num != 0) {
        break;
      }
    }
  }

  // 对指定的组进行解码
  std::vector<AVFrame*> frames{};
  auto frames_release = gsl::finally([&]() {
    for (int i = 0; i < frames.size(); i++) {
      av_frame_free(&frames[i]);
    }
  });
  for (int i = start_gop_num; i <= end_top_num; ++i) {
    // 组装 std::vector<AVPacket*>
    std::vector<AVPacket*> packets;
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    for (const auto& packet_info : bufffer_2[i].packet) {
      AVPacket* temp;
      temp       = av_packet_alloc();
      temp->data = static_cast<uint8_t*>(av_malloc(packet_info.pkg_data.size()));

      memcpy(temp->data, packet_info.pkg_data.data(), packet_info.pkg_data.size());
      temp->size = static_cast<int>(packet_info.pkg_data.size());
      temp->pts  = packet_info.pts;
      temp->dts  = packet_info.dts;
      packets.emplace_back(temp);
    }
    // 解码
    auto temp          = decoder.independent_decoder(bufffer_2[i].extradata, packets);
    for (auto frame : temp) {
      frames.emplace_back(frame);
    }
  }

  // 绘制
  for (auto frame : frames) {
    draw_detection_boxes(frame, get_detections_test());
  }

  // 最后进行编码和保存
  {
    encoding<AVCodecID> encoder(AV_CODEC_ID_H264);
    auto codec_ctx = rtsp_stream.get_stream_ctx()->streams[rtsp_stream.get_video_stream_index()]->codecpar;
    std::vector<uint8_t> extradata(codec_ctx->extradata, codec_ctx->extradata + codec_ctx->extradata_size);
    auto packets         = encoder.get_encode_packets(frames,extradata);
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    // 将编码数据写入文件
    {
      std::ofstream file(output_dir + "output.h264", std::ios::out | std::ios::binary);
      for (auto packet : packets) {
        file.write((char*)packet->data, packet->size);
      }
      file.close();
    }
  }



  return true;
}
