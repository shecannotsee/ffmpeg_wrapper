#include <decode_support.h>
#include <she_test.h>

#include <gsl/util>

#include "encode_support.h"
#include "save_frame_as_jpeg.h"
#include "stream_support.h"
#include "test_support.h"

SHE_TEST(encode_support, encode_from_rtsp) {
  const std::string rtsp_url = "rtsp://localhost:8554/mystream";
  std::string output_dir     = "./encode_support/";
  test_support::create_dir(output_dir);
  output_dir = "./encode_support/rtsp/";
  test_support::create_dir(output_dir);

  stream_info rtsp_stream;
  rtsp_stream.set_format_from(rtsp_url, {});

  decoding<AVCodecID> decoder(rtsp_stream.get_video_type());
  decoder.add_video_param(rtsp_stream.get_video_params());
  decoder.create_decoder();

  std::vector<AVFrame*> frames{};
  auto frames_release = gsl::finally([&]() {
    for (int i = 0; i < frames.size(); i++) {
      av_frame_free(&frames[i]);
    }
  });

  test_support::timer recond;
  recond.start();
  // 获取原始帧数据
  while (true) {
    auto ret = decoder.get_decode_frames(rtsp_stream.get_stream_ctx(), rtsp_stream.get_video_stream_index());
    if (std::get<0>(ret) == decoding<AVCodecID>::state::done) {
      break;
    }
    if (recond.end(); recond.get_elapsed<std::chrono::seconds>() > 5) {
      break;  // 超过五秒退出
    }
    auto temp = std::get<1>(ret);
    for (auto frame : temp) {
      frames.emplace_back(frame);
    }
  }

  // 图片写入测试
  {
    int frame_num = 0;
    for (int i = 0; i < frames.size(); i++) {
      save_frame_as_jpeg(frames[i], frames[i]->width, frames[i]->height,
                         output_dir + std::to_string(++frame_num) + ".jpg");
    }
  }

  // 编码
  {
    encoding<AVCodecID> encoder(AV_CODEC_ID_H264);
    auto codec_ctx = rtsp_stream.get_stream_ctx()->streams[rtsp_stream.get_video_stream_index()]->codecpar;
    std::vector<uint8_t> extradata(codec_ctx->extradata, codec_ctx->extradata + codec_ctx->extradata_size);
    auto packets         = encoder.get_encode_packets(frames, extradata);
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