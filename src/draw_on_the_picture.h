#ifndef DRAW_ON_THE_PICTURE_H
#define DRAW_ON_THE_PICTURE_H

#include <vector>
extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
}

struct detection {
  float x1, y1;
  float x2, y2;
  float confidence;
  int class_id, tracker_id;
};

static std::vector<detection> get_detections_test() {
  return {{478.55, 293.91, 505.92, 316.78, 0.91269, 0, 85}};
}

// Helper function to convert YUVJ420P to RGB24
static void yuv420p_to_rgb24(const AVFrame* frame, AVFrame* rgb_frame) {
  struct SwsContext* sws_ctx = sws_getContext(frame->width, frame->height, AV_PIX_FMT_YUVJ420P, frame->width,
                                              frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

  sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);

  sws_freeContext(sws_ctx);
}

// Function to draw detection boxes on an AVFrame (YUV format)
static void draw_detection_boxes(AVFrame* frame, const std::vector<detection>& detections) {
  // Allocate RGB frame for drawing
  AVFrame* rgb_frame     = av_frame_alloc();
  auto rgp_frame_release = gsl::finally([&]() { av_frame_free(&rgb_frame); });
  if (!rgb_frame) {
    std::cerr << "Error allocating RGB frame" << std::endl;
    return;
  }

  rgb_frame->format = AV_PIX_FMT_RGB24;
  rgb_frame->width  = frame->width;
  rgb_frame->height = frame->height;

  int ret = av_frame_get_buffer(rgb_frame, 32);
  if (ret < 0) {
    std::cerr << "Error allocating RGB frame buffer" << std::endl;
    av_frame_free(&rgb_frame);
    return;
  }

  // Convert YUVJ420P to RGB24
  yuv420p_to_rgb24(frame, rgb_frame);

  // Draw detection boxes on RGB frame
  for (const auto& detection : detections) {
    int x1 = static_cast<int>(detection.x1);
    int y1 = static_cast<int>(detection.y1);
    int x2 = static_cast<int>(detection.x2);
    int y2 = static_cast<int>(detection.y2);

    // Draw rectangle on RGB frame
    for (int x = x1; x <= x2; ++x) {
      rgb_frame->data[0][y1 * rgb_frame->linesize[0] + x * 3]     = 255;  // R
      rgb_frame->data[0][y1 * rgb_frame->linesize[0] + x * 3 + 1] = 0;    // G
      rgb_frame->data[0][y1 * rgb_frame->linesize[0] + x * 3 + 2] = 0;    // B

      rgb_frame->data[0][y2 * rgb_frame->linesize[0] + x * 3]     = 255;  // R
      rgb_frame->data[0][y2 * rgb_frame->linesize[0] + x * 3 + 1] = 0;    // G
      rgb_frame->data[0][y2 * rgb_frame->linesize[0] + x * 3 + 2] = 0;    // B
    }

    for (int y = y1; y <= y2; ++y) {
      rgb_frame->data[0][y * rgb_frame->linesize[0] + x1 * 3]     = 255;  // R
      rgb_frame->data[0][y * rgb_frame->linesize[0] + x1 * 3 + 1] = 0;    // G
      rgb_frame->data[0][y * rgb_frame->linesize[0] + x1 * 3 + 2] = 0;    // B

      rgb_frame->data[0][y * rgb_frame->linesize[0] + x2 * 3]     = 255;  // R
      rgb_frame->data[0][y * rgb_frame->linesize[0] + x2 * 3 + 1] = 0;    // G
      rgb_frame->data[0][y * rgb_frame->linesize[0] + x2 * 3 + 2] = 0;    // B
    }
  }

  // Convert RGB24 back to YUVJ420P
  SwsContext* sws_ctx  = sws_getContext(rgb_frame->width, rgb_frame->height, AV_PIX_FMT_RGB24, frame->width,
                                        frame->height, AV_PIX_FMT_YUVJ420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
  auto sws_ctx_release = gsl::finally([&]() { sws_freeContext(sws_ctx); });

  sws_scale(sws_ctx, rgb_frame->data, rgb_frame->linesize, 0, rgb_frame->height, frame->data, frame->linesize);
}

#endif  // DRAW_ON_THE_PICTURE_H
