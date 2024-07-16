#include <she_test.h>

#include "decode.h"
#include "rtsp_buffer.h"
#include "test_support.h"

SHE_TEST(test, cache_rtsp) {
  // 测试资源
  const std::string rtsp_url   = "rtsp://localhost:8554/mystream";
  const std::string output_dir = "./cache_rtsp";
  test_support::create_dir(output_dir);

  // 开始测试
  rtsp_buffer stream;
  stream.start_buffering(rtsp_url);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  stream.stop_buffering();

  auto bufffer = stream.get_buffer();

  auto count = 0;
  for (auto gop : bufffer) {
    // output_dir/gop_1.h264
    std::string gop_name = output_dir + "/gop_" + std::to_string(++count) + ".h264";
    std::ofstream outfile;
    outfile.open(gop_name);
    if (!outfile) {
      throw std::runtime_error("打开文件失败");
    }
    // 写入数据
    outfile.write(reinterpret_cast<const char*>(gop.extradata.data()), gop.extradata.size());
    for (auto single_frame : gop.packet) {
      outfile.write(reinterpret_cast<const char*>(single_frame.pkg_data.data()), single_frame.pkg_data.size());
    }

    outfile.close();  // 关闭文件
  }

  for (auto gop : bufffer) {
    decode_gop(gop, output_dir);
  }

  return true;
}