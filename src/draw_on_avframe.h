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
 * @brief Color structure for storing RGB and YUV components.
 */
struct color {
  union {
    struct {
      uint8_t r;  ///< Red component (0-255)
      uint8_t g;  ///< Green component (0-255)
      uint8_t b;  ///< Blue component (0-255)
    };
    struct {
      uint8_t y;  ///< Y component (luminance)
      uint8_t u;  ///< U component (chrominance)
      uint8_t v;  ///< V component (chrominance)
    };
  };
};

/**
 * @brief Converts RGB color to YUV color.
 *
 * @param rgb The input RGB color structure.
 * @return The converted YUV color structure.
 */
constexpr color rgb_to_yuv(const color rgb) {
  color yuv{};
  // Convert RGB components to float for calculation
  const float r = rgb.r;
  const float g = rgb.g;
  const float b = rgb.b;

  yuv.y = static_cast<uint8_t>(+0.257f * r + 0.504f * g + 0.098f * b + 16);
  yuv.u = static_cast<uint8_t>(-0.148f * r - 0.291f * g + 0.439f * b + 128);
  yuv.v = static_cast<uint8_t>(+0.439f * r - 0.368f * g - 0.071f * b + 128);
  return yuv;
}

/**
 * @brief Retrieves the corresponding color based on the color name.
 *
 * @param color_name The name of the color. Supported names are "red", "orange", "yellow", "green", "cyan", "blue", "purple", "white", "black".
 * @return The corresponding YUV color structure. If not found, returns red (255, 0, 0).
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
  const auto it = color_map.find(color_name);
  return (it != color_map.end()) ? it->second : color{255, 0, 0};  // Default to red if not found
}

/**
 * @brief Draws a rectangle border on the given AVFrame.
 *
 * @param frame Pointer to the AVFrame to draw on.
 * @param x1 The x-coordinate of the top-left corner of the rectangle.
 * @param y1 The y-coordinate of the top-left corner of the rectangle.
 * @param x2 The x-coordinate of the bottom-right corner of the rectangle.
 * @param y2 The y-coordinate of the bottom-right corner of the rectangle.
 * @param color_name The color name for the border.
 * @param thickness The thickness of the border, default is 4.
 */
void draw(const AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

/**
 * @brief Draws a rectangle border on a NV12 format frame.
 *
 * @param frame Pointer to the AVFrame to draw the rectangle on, must be in NV12 format.
 * @param x1 The x-coordinate of the top-left corner of the rectangle.
 * @param y1 The y-coordinate of the top-left corner of the rectangle.
 * @param x2 The x-coordinate of the bottom-right corner of the rectangle.
 * @param y2 The y-coordinate of the bottom-right corner of the rectangle.
 * @param color_name The color name for the rectangle, supported names are "red", "orange", "yellow", "green", "cyan", "blue", "purple", "white", "black".
 * @param thickness The thickness of the rectangle border, default is 4.
 *
 * @throws std::runtime_error If the provided AVFrame format is not NV12.
 *
 * @note This function directly modifies the Y, U, and V data of the input frame to draw the specified colored rectangle border.
 *       Ensure the AVFrame is properly initialized and its data pointers are valid before calling this function.
 */
void draw_rectangle_nv12(
    AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

/**
 * @brief Draws a line on a NV12 format frame.
 *
 * @param frame Pointer to the AVFrame to draw the line on.
 * @param x1 The x-coordinate of the starting point of the line. This must be an even number.
 * @param y1 The y-coordinate of the starting point of the line.
 * @param x2 The x-coordinate of the ending point of the line.
 * @param y2 The y-coordinate of the ending point of the line.
 * @param color_name The color name for the line.
 * @param thickness The thickness of the line, default is 4.
 */
void draw_line_nv12(const AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, int thickness = 4);

/**
 * @brief Draws an area (polygon) on a NV12 format frame.
 *
 * @param frame Pointer to the AVFrame to draw the area on.
 * @param area A vector of points that defines the area.
 * @param color_name The color name for the area.
 * @param thickness The thickness of the border, default is 4.
 */
void draw_area_nv12(AVFrame* frame, const std::vector<point>& area, const std::string& color_name, int thickness = 4);

/**
 * @brief Draws a digit on a NV12 format frame.
 *
 * @param frame Pointer to the AVFrame to draw the digit on.
 * @param x The x-coordinate to start drawing the digit.
 * @param y The y-coordinate to start drawing the digit.
 * @param digit The digit to draw.
 * @param color_name The color name for the digit.
 * @param thickness The thickness of the digit lines, default is 1.
 */
void draw_digit_nv12(AVFrame* frame, int x, int y, int digit, const std::string& color_name, int thickness = 1);

/**
 * @brief Draws a letter on a NV12 format frame.
 *
 * @param frame Pointer to the AVFrame to draw the letter on.
 * @param x The x-coordinate to start drawing the letter.
 * @param y The y-coordinate to start drawing the letter.
 * @param letter The letter to draw.
 * @param color_name The color name for the letter.
 * @param thickness The thickness of the letter lines, default is 1.
 */
void draw_letter_nv12(
    const AVFrame* frame, int x, int y, char letter, const std::string& color_name, int thickness = 1);

#endif  // FFMPEG_WRAPPER_DRAW_ON_AVFRAME_H
