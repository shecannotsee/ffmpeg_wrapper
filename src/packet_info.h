//
// Created by shecannotsee on 24-6-14.
//

#ifndef PACKET_INFO_H
#define PACKET_INFO_H

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace packet_info {
/**
 * @brief 获取解码时间
 * @param frame
 * @return
 */
inline auto get_dts(const AVPacket* frame) -> int64_t {
  return frame->dts;
}

/**
 * @brief 获取显示时间
 * @param frame
 * @return
 */
inline auto get_pts(const AVPacket* frame) -> int64_t {
  return frame->pts;
}

}  // namespace packet_info

#endif //PACKET_INFO_H
