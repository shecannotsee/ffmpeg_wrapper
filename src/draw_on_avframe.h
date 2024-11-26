#ifndef FFMPEG_WRAPPER_DRAW_ON_AVFRAME_H
#define FFMPEG_WRAPPER_DRAW_ON_AVFRAME_H
#include "frame_support.h"

extern "C" {
#include <libavutil/frame.h>
}

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * @brief 颜色结构体，用于存储 RGB 和 YUV 分量。
 */
struct color {
  union {
    struct {
      uint8_t r;  ///< 红色分量 (0-255)
      uint8_t g;  ///< 绿色分量 (0-255)
      uint8_t b;  ///< 蓝色分量 (0-255)
    };
    struct {
      uint8_t y;  ///< Y 分量 (亮度)
      uint8_t u;  ///< U 分量 (色度)
      uint8_t v;  ///< V 分量 (色度)
    };
  };
};

/**
 * @brief 将 RGB 颜色转换为 YUV 颜色。
 *
 * @param rgb 输入的 RGB 颜色结构体。
 * @return 转换后的 YUV 颜色结构体。
 */
constexpr color rgb_to_yuv(color rgb) {
  color yuv{};
  // 需要将 RGB 分量转换为浮点数进行计算
  float r = rgb.r;
  float g = rgb.g;
  float b = rgb.b;

  yuv.y = static_cast<uint8_t>(+0.257f * r + 0.504f * g + 0.098f * b + 16);
  yuv.u = static_cast<uint8_t>(-0.148f * r - 0.291f * g + 0.439f * b + 128);
  yuv.v = static_cast<uint8_t>(+0.439f * r - 0.368f * g - 0.071f * b + 128);
  return yuv;
}

/**
 * @brief 根据颜色名称获取对应的颜色。
 *
 * @param color_name 颜色名称，支持的名称包括 "red", "orange", "yellow", "green", "cyan", "blue", "purple", "white",
 * "black"。
 * @return 对应的 YUV 颜色结构体，如果未找到，则返回红色 (255, 0, 0)。
 */
static color get_color(const std::string& color_name) {
  static std::unordered_map<std::string, color> color_map = {
      {"red", rgb_to_yuv({255, 0, 0})},        // YUV( 76,  84, 255) RGB (255,   0,   0)
      {"orange", rgb_to_yuv({255, 128, 0})},   // YUV(151,  42, 201) RGB (255, 128,   0)
      {"yellow", rgb_to_yuv({255, 255, 0})},   // YUV{225,   0, 148} RGB {255, 255,   0}
      {"green", rgb_to_yuv({0, 255, 0})},      // YUV{149,  43,  21} RGB {  0, 255,   0}
      {"cyan", rgb_to_yuv({0, 255, 255})},     // YUV{178, 171,   0} RGB {  0, 255, 255}
      {"blue", rgb_to_yuv({0, 0, 255})},       // YUV{ 29, 255, 107} RGB {  0,   0, 255}
      {"purple", rgb_to_yuv({255, 0, 255})},   // YUV{105, 212, 234} RGB {255,   0, 255}
      {"white", rgb_to_yuv({255, 255, 255})},  // YUV{255, 128, 128} RGB {255, 255, 255}
      {"black", rgb_to_yuv({0, 0, 0})},        // YUV{  0, 128, 128} RGB {  0,   0,   0}
  };
  auto it = color_map.find(color_name);
  return (it != color_map.end()) ? it->second : color{255, 0, 0};  // 默认红色
}

/**
 * @brief 在给定的 AVFrame 上绘制矩形边框。
 *
 * @param frame 指向要绘制的 AVFrame 的指针。
 * @param x1 矩形左上角 x 坐标。
 * @param y1 矩形左上角 y 坐标。
 * @param x2 矩形右下角 x 坐标。
 * @param y2 矩形右下角 y 坐标。
 * @param color_name 边框的颜色名称。
 * @param thickness 边框的厚度，默认为 4。
 */
void draw(AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

/**
 * @brief 在 NV12 格式的帧上绘制矩形边框。
 *
 * @param frame 指向要绘制矩形的 AVFrame 结构体指针，必须为 NV12 格式。
 * @param x1 矩形左上角的 x 坐标。
 * @param y1 矩形左上角的 y 坐标。
 * @param x2 矩形右下角的 x 坐标。
 * @param y2 矩形右下角的 y 坐标。
 * @param color_name 颜色名称，支持的名称包括 "red", "orange", "yellow", "green", "cyan", "blue", "purple", "white",
 * "black"。
 * @param thickness 边框的厚度，默认值为 4。
 *
 * @throws std::runtime_error 如果提供的 AVFrame 格式不是 NV12。
 *
 * @note 此函数将直接修改输入帧的 Y、U 和 V 数据以绘制指定颜色的矩形边框。
 *       请确保在调用该函数之前，AVFrame 已经正确初始化并且数据指针有效。
 */
void draw_rectangle_nv12(
    AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

/**
 * @brief
 * @param frame
 * @param x1 一定要为偶数
 * @param y1
 * @param x2
 * @param y2
 * @param color_name
 * @param thickness
 */
void draw_line_nv12(AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

void draw_area_nv12(AVFrame* frame, std::vector<point> area, const std::string& color_name, int thickness = 4);

void draw_digit_nv12(AVFrame* frame, int x1, int y1, int digit, const std::string& color_name, int thickness = 1);

void draw_letter_nv12(AVFrame* frame, int x, int y, char letter, const std::string& color_name, int thickness = 1);

#endif  // FFMPEG_WRAPPER_DRAW_ON_AVFRAME_H
