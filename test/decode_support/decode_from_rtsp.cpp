#include <decode_support.h>
#include <she_log.h>
#include <she_test.h>

#include <gsl/util>

#include "save_frame_as_jpeg.h"
#include "test_support.h"

SHE_TEST(decode_support, decode_from_rtsp) {
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./decode_support/rtsp/";
  test_support::create_dir(output_dir);

  pre_decoding rtsp_stream;
  rtsp_stream.set_format_from(rtsp_url);

  decoding<AVCodecID> decoder(rtsp_stream.get_video_type());
  decoder.add_video_param(rtsp_stream.get_video_params());
  decoder.create_decoder();

  int frame_num = 0;
  while (true) {
    auto ret = decoder.get_decode_frames(rtsp_stream.get_stream_ctx(), rtsp_stream.get_video_stream_index());
    if (std::get<0>(ret) == decoding<AVCodecID>::state::done) {
      break;
    }
    auto temp         = std::get<1>(ret);
    auto temp_release = gsl::finally([&]() {
      for (int i = 0; i < temp.size(); i++) {
        av_frame_free(&temp[i]);
      }
    });

    for (int i = 0; i < temp.size(); i++) {
      save_frame_as_jpeg(temp[i], temp[i]->width, temp[i]->height, output_dir + std::to_string(++frame_num) + ".jpg");
    }
  }

  return true;
}