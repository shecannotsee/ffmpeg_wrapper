#include "draw_on_avframe.h"

#include <stdexcept>

extern "C" {
#include <libavutil/imgutils.h>
}

void draw(const AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, const int thickness) {
  const color draw_color = get_color(color_name);

  // YUV420P格式的宽度和高度
  const int width  = frame->width;
  const int height = frame->height;

  // 限制坐标在图像边界内
  x1 = std::max(0, std::min(x1, width - 1));
  x2 = std::max(0, std::min(x2, width - 1));
  y1 = std::max(0, std::min(y1, height - 1));
  y2 = std::max(0, std::min(y2, height - 1));

  // 获取Y、U和V分量数据
  uint8_t* y_plane = frame->data[0];
  uint8_t* u_plane = frame->data[1];
  uint8_t* v_plane = frame->data[2];

  const int y_line_size = frame->linesize[0];
  const int u_line_size = frame->linesize[1];
  const int v_line_size = frame->linesize[2];

  // 绘制顶部和底部边框
  for (int x = x1; x <= x2; ++x) {
    for (int t = 0; t < thickness; ++t) {
      if (y1 - t >= 0) {
        y_plane[(y1 - t) * y_line_size + x] = draw_color.y;
        if ((x % 2 == 0) && ((y1 - t) % 2 == 0)) {
          u_plane[(y1 - t) / 2 * u_line_size + x / 2] = draw_color.u;
          v_plane[(y1 - t) / 2 * v_line_size + x / 2] = draw_color.v;
        }
      }
      if (y2 + t < height) {
        y_plane[(y2 + t) * y_line_size + x] = draw_color.y;
        if ((x % 2 == 0) && ((y2 + t) % 2 == 0)) {
          u_plane[(y2 + t) / 2 * u_line_size + x / 2] = draw_color.u;
          v_plane[(y2 + t) / 2 * v_line_size + x / 2] = draw_color.v;
        }
      }
    }
  }

  // 绘制左侧和右侧边框
  for (int y = y1; y <= y2; ++y) {
    for (int t = 0; t < thickness; ++t) {
      if (x1 - t >= 0) {
        y_plane[y * y_line_size + (x1 - t)] = draw_color.y;
        if ((x1 - t) % 2 == 0 && y % 2 == 0) {
          u_plane[y / 2 * u_line_size + (x1 - t) / 2] = draw_color.u;
          v_plane[y / 2 * v_line_size + (x1 - t) / 2] = draw_color.v;
        }
      }
      if (x2 + t < width) {
        y_plane[y * y_line_size + (x2 + t)] = draw_color.y;
        if ((x2 + t) % 2 == 0 && y % 2 == 0) {
          u_plane[y / 2 * u_line_size + (x2 + t) / 2] = draw_color.u;
          v_plane[y / 2 * v_line_size + (x2 + t) / 2] = draw_color.v;
        }
      }
    }
  }
}

void draw_rectangle_nv12(
    AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, const int thickness) {
  const auto format = static_cast<enum AVPixelFormat>(frame->format);

  if (frame->format != AV_PIX_FMT_NV12) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }

  const color draw_color = get_color(color_name);

  // NV12格式的宽度和高度
  const int width  = frame->width;
  const int height = frame->height;

  // 限制坐标在图像边界内
  x1 = std::max(0, std::min(x1, width - 1));
  x2 = std::max(0, std::min(x2, width - 1));
  y1 = std::max(0, std::min(y1, height - 1));
  y2 = std::max(0, std::min(y2, height - 1));

  // 获取Y、U和V分量数据
  uint8_t* y_plane  = frame->data[0];
  uint8_t* uv_plane = frame->data[1];

  int y_line_size  = frame->linesize[0];
  int uv_line_size = frame->linesize[1];

  // 绘制顶部和底部边框
  for (int x = x1; x <= x2; ++x) {
    for (int t = 0; t < thickness; ++t) {
      if (y1 - t >= 0) {
        y_plane[(y1 - t) * y_line_size + x] = draw_color.y;

        // 设置U和V分量，注意每两个像素共享一个UV值
        if (x % 2 == 0) {
          int uv_index           = (y1 - t) / 2 * uv_line_size + x;  // NV12格式: U 和 V 交替存储
          uv_plane[uv_index]     = draw_color.u;
          uv_plane[uv_index + 1] = draw_color.v;
        }
      }
      if (y2 + t < height) {
        y_plane[(y2 + t) * y_line_size + x] = draw_color.y;
        if (x % 2 == 0) {
          int uv_index           = (y2 + t) / 2 * uv_line_size + x;
          uv_plane[uv_index]     = draw_color.u;
          uv_plane[uv_index + 1] = draw_color.v;
        }
      }
    }
  }

  // 绘制左侧和右侧边框
  for (int y = y1; y <= y2; ++y) {
    for (int t = 0; t < thickness; ++t) {
      if (x1 - t >= 0) {
        y_plane[y * y_line_size + (x1 - t)] = draw_color.y;
        if ((x1 - t) % 2 == 0) {
          int uv_index           = y / 2 * uv_line_size + (x1 - t);
          uv_plane[uv_index]     = draw_color.u;
          uv_plane[uv_index + 1] = draw_color.v;
        }
      }
      if (x2 + t < width) {
        y_plane[y * y_line_size + (x2 + t)] = draw_color.y;
        if ((x2 + t) % 2 == 0) {
          int uv_index           = y / 2 * uv_line_size + (x2 + t);
          uv_plane[uv_index]     = draw_color.u;
          uv_plane[uv_index + 1] = draw_color.v;
        }
      }
    }
  }
}

// 绘制线段的辅助函数
void draw_line_nv12(const AVFrame* frame, int x1, int y1, int x2, int y2, const std::string& color_name, const int thickness) {
  color draw_color = get_color(color_name);

  int width  = frame->width;
  int height = frame->height;

  uint8_t* y_plane  = frame->data[0];
  uint8_t* uv_plane = frame->data[1];

  int y_line_size  = frame->linesize[0];
  int uv_line_size = frame->linesize[1];

  // 使用 Bresenham 算法绘制线段
  int dx  = std::abs(x2 - x1);
  int dy  = std::abs(y2 - y1);
  int sx  = (x1 < x2) ? 1 : -1;
  int sy  = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    // 限制坐标在图像边界内
    if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
      for (int t = 0; t < thickness; ++t) {
        int yt = y1 + t;
        if (yt < height) {
          y_plane[yt * y_line_size + x1] = draw_color.y;
          if (x1 % 2 == 0) {
            int uv_index           = (yt / 2) * uv_line_size + x1;
            uv_plane[uv_index]     = draw_color.u;
            uv_plane[uv_index + 1] = draw_color.v;
          }
        }
      }
    }

    if (x1 == x2 && y1 == y2) break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }
}

void draw_area_nv12(AVFrame* frame, std::vector<point> area, const std::string& color_name, int thickness) {
  const auto format = static_cast<enum AVPixelFormat>(frame->format);

  if (frame->format != AV_PIX_FMT_NV12) {
    std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }

  color draw_color = get_color(color_name);

  const int num_points = area.size();
  if (num_points < 3) {
    throw std::runtime_error("A polygon requires at least 3 points");
  }

  // 绘制多边形的边
  for (int i = 0; i < num_points; ++i) {
    int x1 = static_cast<int>(std::round(area[i].x));
    int y1 = static_cast<int>(std::round(area[i].y));
    int x2 = static_cast<int>(std::round(area[(i + 1) % num_points].x));
    int y2 = static_cast<int>(std::round(area[(i + 1) % num_points].y));

    draw_line_nv12(frame, x1, y1, x2, y2, color_name, thickness);
  }
}

void draw_digit_nv12(AVFrame* frame, int x, int y, int digit, const std::string& color_name, int thickness) {
  if (x % 2 != 0) x += 1;
  if (y % 2 != 0) y += 1;
  // 10-20-15,6-12-10
  constexpr int xm               = 6;   ///< digit x max offset
  constexpr int ym               = 12;  ///< digit y max offset
  constexpr int default_x_offset = 10;  ///<
  // point: Top-Middle-Bottom, Left-Right
  // TL(0,0)    TR(xm,0)
  // ML(0,ym/2) MR(xm,ym/2)
  // BL(0,ym)   BR(xm,ym)
  constexpr struct {
    int x;
    int y;
  } TL{0, 0}, TR{xm, 0}, ML{0, ym / 2}, MR{xm, ym / 2}, BL{0, ym}, BR{xm, ym};

  // 保证不会越界
  int max_x = frame->width - xm;   // x 最大值
  int max_y = frame->height - ym;  // y 最大值
  // 限制 x 和 y 在合法范围内
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > max_x) x = max_x;
  if (y > max_y) y = max_y;
  switch (digit) {
    case 0: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, BR.x + x, BR.y + y, BL.x + x, BL.y + y, color_name, thickness);
      draw_line_nv12(frame, BL.x + x, BL.y + y, TL.x + x, TL.y + y, color_name, thickness);
      return;
    }
    case 1: {
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      return;
    }
    case 2: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, MR.x + x, MR.y + y, color_name, thickness);
      draw_line_nv12(frame, MR.x + x, MR.y + y, ML.x + x, ML.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, BL.x + x, BL.y + y, color_name, thickness);
      draw_line_nv12(frame, BL.x + x, BL.y + y, BR.x + x, BR.y + y, color_name, thickness);
      return;
    }
    case 3: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      draw_line_nv12(frame, BL.x + x, BL.y + y, BR.x + x, BR.y + y, color_name, thickness);
      return;
    }
    case 4: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, ML.x + x, ML.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      return;
    }
    case 5: {
      draw_line_nv12(frame, TR.x + x, TR.y + y, TL.x + x, TL.y + y, color_name, thickness);
      draw_line_nv12(frame, TL.x + x, TL.y + y, ML.x + x, ML.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      draw_line_nv12(frame, MR.x + x, MR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, BR.x + x, BR.y + y, BL.x + x, BL.y + y, color_name, thickness);
      return;
    }
    case 6: {
      draw_line_nv12(frame, TR.x + x, TR.y + y, TL.x + x, TL.y + y, color_name, thickness);
      draw_line_nv12(frame, TL.x + x, TL.y + y, BL.x + x, BL.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      draw_line_nv12(frame, MR.x + x, MR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, BR.x + x, BR.y + y, BL.x + x, BL.y + y, color_name, thickness);
      return;
    }
    case 7: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      return;
    }
    case 8: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, BR.x + x, BR.y + y, BL.x + x, BL.y + y, color_name, thickness);
      draw_line_nv12(frame, BL.x + x, BL.y + y, TL.x + x, TL.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      return;
    }
    case 9: {
      draw_line_nv12(frame, TL.x + x, TL.y + y, TR.x + x, TR.y + y, color_name, thickness);
      draw_line_nv12(frame, TR.x + x, TR.y + y, BR.x + x, BR.y + y, color_name, thickness);
      draw_line_nv12(frame, BR.x + x, BR.y + y, BL.x + x, BL.y + y, color_name, thickness);
      draw_line_nv12(frame, TL.x + x, TL.y + y, ML.x + x, ML.y + y, color_name, thickness);
      draw_line_nv12(frame, ML.x + x, ML.y + y, MR.x + x, MR.y + y, color_name, thickness);
      return;
    }
    default:
      // 使用递归处理多位数，横向偏移default_x_offset个单位
      std::string digits = std::to_string(digit);
      int offset         = 0;
      for (char c : digits) {
        int current_digit = c - '0';  // 将字符转换为整数
        draw_digit_nv12(frame, x + offset, y, current_digit, color_name, thickness);
        offset += default_x_offset;
      }
      return;
  }
}

void draw_letter_nv12(AVFrame* frame, int x, int y, char letter, const std::string& color_name, int thickness) {
  if (x % 2 != 0) x += 1;
  if (y % 2 != 0) y += 1;

  switch (letter) {
    case 'A':
      draw_line_nv12(frame, 6 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 4 + x, 12 + y, 8 + x, 12 + y, color_name, thickness);
      return;
    case 'B':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 6 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 6 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 6 + x, 24 + y, color_name, thickness);

      draw_line_nv12(frame, 6 + x, 0 + y, 12 + x, 4 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 4 + y, 12 + x, 8 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 8 + y, 6 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 12 + y, 12 + x, 16 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 16 + y, 12 + x, 20 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 20 + y, 6 + x, 24 + y, color_name, thickness);
      return;
    case 'C':
      draw_line_nv12(frame, 12 + x, 0 + y, 6 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 0 + x, 6 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 6 + y, 0 + x, 18 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 18 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'D':
      draw_line_nv12(frame, 0 + x, 0 + y, 6 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 12 + x, 6 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 6 + y, 12 + x, 18 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 18 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 0 + x, 0 + y, color_name, thickness);
      return;
    case 'E':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      return;
    case 'F':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      return;
    case 'G':
      draw_line_nv12(frame, 12 + x, 0 + y, 0 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 12 + y, 6 + x, 12 + y, color_name, thickness);
      return;
    case 'H':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 12 + y, color_name, thickness);
      return;
    case 'I':
      draw_line_nv12(frame, 6 + x, 0 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 4 + x, 0 + y, 8 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 4 + x, 24 + y, 8 + x, 24 + y, color_name, thickness);
      return;
    case 'J':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 0 + x, 18 + y, color_name, thickness);
      return;
    case 'K':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 0 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'L':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'M':
      draw_line_nv12(frame, 0 + x, 24 + y, 0 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'N':
      draw_line_nv12(frame, 0 + x, 24 + y, 0 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 12 + x, 0 + y, color_name, thickness);
      return;
    case 'O':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 0 + x, 0 + y, color_name, thickness);
      return;
    case 'P':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 12 + y, 0 + x, 12 + y, color_name, thickness);
      return;
    case 'Q':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 12 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 0 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 12 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'R':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 12 + y, 0 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    case 'S':
      draw_line_nv12(frame, 12 + x, 0 + y, 0 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 12 + y, 12 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 12 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 0 + x, 24 + y, color_name, thickness);
      return;
    case 'T':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 6 + x, 24 + y, color_name, thickness);
      return;
    case 'U':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 12 + x, 0 + y, color_name, thickness);
      return;
    case 'V':
      draw_line_nv12(frame, 0 + x, 0 + y, 6 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 24 + y, 12 + x, 0 + y, color_name, thickness);
      return;
    case 'W':
      draw_line_nv12(frame, 0 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 6 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 24 + y, 12 + x, 0 + y, color_name, thickness);
      return;
    case 'X':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      return;
    case 'Y':
      draw_line_nv12(frame, 0 + x, 0 + y, 6 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 6 + x, 12 + y, color_name, thickness);
      draw_line_nv12(frame, 6 + x, 12 + y, 6 + x, 24 + y, color_name, thickness);
      return;
    case 'Z':
      draw_line_nv12(frame, 0 + x, 0 + y, 12 + x, 0 + y, color_name, thickness);
      draw_line_nv12(frame, 12 + x, 0 + y, 0 + x, 24 + y, color_name, thickness);
      draw_line_nv12(frame, 0 + x, 24 + y, 12 + x, 24 + y, color_name, thickness);
      return;
    default:
      std::string error_message = "Not support letter: " + std::string(1, letter);
      throw std::runtime_error(error_message);
  }
}
