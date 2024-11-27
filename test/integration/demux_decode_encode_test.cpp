#include <gtest/gtest.h>

#include "av_codec_context_support.h"
#include "decode.h"
#include "demux.h"
#include "encode.h"
#include "image_support.h"
#include "test_support.h"

namespace {
namespace resource {
const std::string url        = "../resource/NeroAVC.mp4";
const auto stream_params     = std::vector<std::tuple<std::string, std::string, int>>{};
const std::string output_dir = "../resource/demux_decode_encode_test";
const std::string file_name  = "save.h264";
}  // namespace resource

}  // namespace

TEST(functional, demux_decode_encode_test) {
  test_support::create_dir(resource::output_dir);

  std::cout << "Use demux open " + resource::url << std::endl;
  demux stream;
  stream.open(resource::url, resource::stream_params);

  av_codec_context_support ctx;
  const auto id = stream.get_codec_id<demux::type::video>();
  decode decoder(id);
  decoder.create_decoder(ctx.add_avcodec_parameters<demux::type::video>(stream));

  encode encoder(id);
  encoder.create_encoder(ctx.get_h264_encode(stream));

  constexpr int max_frames_sum = 250;
  std::cout << "Start receiving " << max_frames_sum << " frames ......\n";

  int count = 0;
  for (int i = 0; i < max_frames_sum; i++) {
    auto pkts = stream.start_receiving<demux::type::video>(1);
    for (auto &[type, pkt] : pkts) {
      auto frames = decoder.decoding(pkt);  // 对数据包进行解码
      for (auto &frame : frames) {
        std::string file_name = "./" + resource::output_dir + "/" + std::to_string(++count) + ".pgm";
        pgm_save(frame, file_name);
        jpeg_save(frame, file_name);
        auto en_pkts                     = encoder.encoding(frame);
        const std::string save_file_name = "./" + resource::output_dir + "/" + resource::file_name;
        for (auto en_pkt : en_pkts) {
          test_support::write_to_file(save_file_name, reinterpret_cast<char *>(en_pkt.get()->data), en_pkt.get()->size,
                                      true);
        }
      }
    }

  }
}
