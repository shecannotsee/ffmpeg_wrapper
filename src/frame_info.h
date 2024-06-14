#ifndef FRAME_INFO_H
#define FRAME_INFO_H

#include <string>

extern "C" {
#include <libavutil/frame.h>
}

namespace frame_info {

inline auto get_type(const AVFrame* frame) -> std::string {
  return (frame->pict_type == AV_PICTURE_TYPE_I)   ? std::string("I")
         : (frame->pict_type == AV_PICTURE_TYPE_P) ? std::string("P")
         : (frame->pict_type == AV_PICTURE_TYPE_B) ? std::string("B")
                                                   : std::string("?");
}

/**
 * @brief 获取解码时间
 * @param frame
 * @return
 */
inline auto get_dts(const AVFrame* frame) -> int64_t {
  return frame->pkt_dts;
}

/**
 * @brief 获取显示时间
 * @param frame
 * @return
 */
inline auto get_pts(const AVFrame* frame) -> int64_t {
  return frame->pts;
}

}  // namespace frame_info

#endif  // FRAME_INFO_H
