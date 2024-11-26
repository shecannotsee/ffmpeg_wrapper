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
 * @brief 检测框信息的结构体
 *
 * 该结构体包含了检测框的坐标、置信度和分类信息，用于表示图像中的一个检测对象。
 */
struct detection {
  float x1;            ///< 检测框左上角的x坐标
  float y1;            ///< 检测框左上角的y坐标
  float x2;            ///< 检测框右下角的x坐标
  float y2;            ///< 检测框右下角的y坐标
  float confidence;    ///< 检测框的置信度
  int class_id;        ///< 检测框的类别ID
  int tracker_id{-1};  ///< 跟踪器ID，用于标识跟踪目标
};

/**
 * @brief 表示多边形区域的结构体
 *
 * 该结构体使用点的集合来表示一个区域，可以用于指定检测或分析的特定区域。
 */
struct area {
  std::vector<point> points;  ///< 构成区域的顶点集合
  void append_point(const point& new_point) {
    points.emplace_back(new_point);
  }
};

/**
 * @brief 保存证据信息的结构体
 *
 * 该结构体包含了一个帧的相关信息，包括时间戳、帧号、检测框集合和区域集合等。
 */
struct save_evidence_info {
  int pts;                       ///< 帧的时间戳（Presentation Time Stamp）
  int frame_number;              ///< 帧号，用于标识帧的顺序
  std::vector<detection> boxes;  ///< 检测框集合，用于存储在该帧中检测到的对象

  /**
   * @brief 向检测框集合添加一个检测框
   *
   * @param box 要添加的检测框
   */
  void append_box(const detection& box) {
    boxes.emplace_back(box);
  }

  std::vector<area> areas;  ///< 区域集合，用于定义该帧中需要分析的区域

  /**
   * @brief 向区域集合添加一个区域
   *
   * @param parea 要添加的区域
   */
  void append_area(const area& new_area) {
    areas.emplace_back(new_area);
  }
};

#endif  // FFMPEG_WRAPPER_FRAME_SUPPORT_H
