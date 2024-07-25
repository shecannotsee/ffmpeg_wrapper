#include <decode_support.h>
#include <she_log.h>
#include <she_test.h>

#include <gsl/util>

#include "save_frame_as_jpeg.h"
#include "test_support.h"

SHE_TEST(decode_support, decode_from_file) {
  const std::string file_path  = "../resource/KTMR2R.mp4";
  const std::string output_dir = "./decode_support/";
  test_support::create_dir(output_dir);

  pre_decoding mp4_file;
  mp4_file.set_format_from(file_path);

  decoding<AVCodecID> decoder(mp4_file.get_video_type());
  decoder.add_video_param(mp4_file.get_video_params());
  decoder.create_decoder();

  int frame_num = 0;
  while (true) {
    auto ret = decoder.get_decode_frames(mp4_file.get_stream_ctx(), mp4_file.get_video_stream_index());
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