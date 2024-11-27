#ifndef FFMPEG_WRAPPER_FRAME_SUPPORT_H
#define FFMPEG_WRAPPER_FRAME_SUPPORT_H

#include <vector>

/**
 * @brief 表示二维平面上点的结构体
 *
 * 该结构体定义了二维平面上的一个点，用x和y表示坐标。
 */
struct point {
  float x;  ///< 点的x坐标
  float y;  ///< 点的y坐标
};

/**
 * @brief 表示多边形区域的结构体
 *
 * 该结构体使用点的集合来表示一个区域，可以用于指定检测或分析的特定区域。
 */
struct area {
  std::vector<point> points;  ///< 构成区域的顶点集合
};

#endif  // FFMPEG_WRAPPER_FRAME_SUPPORT_H
