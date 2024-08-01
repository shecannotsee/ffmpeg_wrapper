//
// Created by shecannotsee on 24-7-12.
//

#ifndef GOP_SEGMENT_H
#define GOP_SEGMENT_H

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <cstdint>
#include <vector>

using bytes = std::vector<uint8_t>;  ///< 字节流,长度内置

// 帧信息
struct av_packet_info {
  int64_t pts{0};    ///< 用来回溯的标识
  int64_t dts{0};
  bytes pkg_data{};  ///< 实际缓存的数据
};

struct gop_segment {
  bytes extradata{};                ///< 只有关键帧才添加的SPS 和 PPS 数据,每个gop只有一个
  std::vector<av_packet_info> packet{};  ///< 实际缓存的数据
  int64_t earliest_pts{0};    ///< 每一个gop中最早的pts,主要用来快速筛选
  int64_t earliest_dts{0};    ///< 每一个gop中最早的dts,主要用来快速筛选
};

#endif  // GOP_SEGMENT_H
