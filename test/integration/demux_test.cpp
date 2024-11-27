#include "demux.h"

#include <gtest/gtest.h>

#include "test_support.h"

namespace {
namespace resource {
const std::string url        = "../resource/test.h264";
const auto stream_params     = std::vector<std::tuple<std::string, std::string, int>>{};
const std::string output_dir = "../resource/demux_test";
const std::string file_name  = "save.h264";
}  // namespace resource

}  // namespace

TEST(unit, demux_test) {
  test_support::create_dir(resource::output_dir);

  std::cout << "Use demux open " + resource::url << std::endl;
  demux stream;
  stream.open(resource::url, resource::stream_params);

  constexpr int max_frames_sum = 250;
  std::cout << "Start receiving " << max_frames_sum << " frames ......\n";

  int count                        = 0;
  const std::string save_file_name = "./" + resource::output_dir + "/" + resource::file_name;
  for (int i = 0; i < max_frames_sum; i++) {
    auto pkts = stream.start_receiving<demux::type::video>(1);
    for (auto& [type, pkt] : pkts) {
      if (++count % 25 == 0) {
        std::cout << "pkt size: " << pkt.get()->size << ", count: " << count << std::endl;
      }
      test_support::write_to_file(save_file_name, reinterpret_cast<char*>(pkt.get()->data), pkt.get()->size, true);
    }
  }
  std::cout << "packet size: " << count << std::endl;
  std::cout << "The file is saved in path: "      //
            << resource::output_dir               // dir
            << "/"                                //
            << resource::file_name << std::endl;  // file
}
