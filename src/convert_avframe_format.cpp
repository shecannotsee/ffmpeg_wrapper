#include "convert_avframe_format.h"

#include <stdexcept>
#include <string>

extern "C" {
#include <libavutil/pixdesc.h>
}

namespace {
auto nv12_to_yuv420p_sample(AVFrame* nv12) -> AVFrame* {
  const auto format = static_cast<enum AVPixelFormat>(nv12->format);

  if (nv12->format != AV_PIX_FMT_NV12) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }
  AVFrame* yuv420p = av_frame_alloc();  // Need to check the return value
  yuv420p->format  = AV_PIX_FMT_YUV420P;
  yuv420p->width   = nv12->width;
  yuv420p->height  = nv12->height;
  av_frame_get_buffer(yuv420p, 32);  // Need to check the return value
  av_frame_make_writable(yuv420p);   // Need to check the return value

  // Y
  if (nv12->linesize[0] == nv12->width) {
    memcpy(yuv420p->data[0], nv12->data[0], nv12->height * nv12->linesize[0]);
  } else {
    for (int y = 0; y < yuv420p->height; y++) {
      for (int x = 0; x < yuv420p->width; x++) {
        yuv420p->data[0][y * yuv420p->linesize[0] + x] = nv12->data[0][y * nv12->linesize[0] + x];
      }
    }
  }
  // Cb and Cr
  for (int y = 0; y < yuv420p->height / 2; y++) {
    for (int x = 0; x < yuv420p->width / 2; x++) {
      yuv420p->data[1][y * yuv420p->linesize[1] + x] = nv12->data[1][y * nv12->linesize[1] + 2 * x];
      yuv420p->data[2][y * yuv420p->linesize[2] + x] = nv12->data[1][y * nv12->linesize[1] + 2 * x + 1];
    }
  }

  return yuv420p;
}
}  // namespace
void nv12_to_yuv420p(av_frame& nv12) {
  const auto format = static_cast<enum AVPixelFormat>(nv12.get()->format);

  if (nv12.get()->format != AV_PIX_FMT_NV12) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }
  av_frame yuv420p;
  yuv420p.get()->format = AV_PIX_FMT_YUV420P;
  yuv420p.get()->width  = nv12.get()->width;
  yuv420p.get()->height = nv12.get()->height;
  yuv420p.allocate_buffer(32);
  yuv420p.make_writable();

  // Y
  if (nv12.get()->linesize[0] == nv12.get()->width) {
    memcpy(yuv420p.get()->data[0], nv12.get()->data[0], nv12.get()->height * nv12.get()->linesize[0]);
  } else {
    for (int y = 0; y < yuv420p.get()->height; y++) {
      for (int x = 0; x < yuv420p.get()->width; x++) {
        yuv420p.get()->data[0][y * yuv420p.get()->linesize[0] + x] =
            nv12.get()->data[0][y * nv12.get()->linesize[0] + x];
      }
    }
  }
  // Cb and Cr
  for (int y = 0; y < yuv420p.get()->height / 2; y++) {
    for (int x = 0; x < yuv420p.get()->width / 2; x++) {
      yuv420p.get()->data[1][y * yuv420p.get()->linesize[1] + x] =
          nv12.get()->data[1][y * nv12.get()->linesize[1] + 2 * x];
      yuv420p.get()->data[2][y * yuv420p.get()->linesize[2] + x] =
          nv12.get()->data[1][y * nv12.get()->linesize[1] + 2 * x + 1];
    }
  }

  nv12 = std::move(yuv420p);
}

namespace {
auto yuv420p_to_nv12_sample(AVFrame* yuv420p) -> AVFrame* {
  const auto format = static_cast<enum AVPixelFormat>(yuv420p->format);

  if (yuv420p->format != AV_PIX_FMT_YUV420P) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }
  AVFrame* nv12 = av_frame_alloc();  // Need to check the return value
  nv12->format  = AV_PIX_FMT_NV12;
  nv12->width   = yuv420p->width;
  nv12->height  = yuv420p->height;
  av_frame_get_buffer(nv12, 32);  // Need to check the return value
  av_frame_make_writable(nv12);   // Need to check the return value

  // Y
  if (yuv420p->linesize[0] == yuv420p->width) {
    memcpy(nv12->data[0], yuv420p->data[0], yuv420p->height * yuv420p->linesize[0]);
  } else {
    for (int y = 0; y < nv12->height; y++) {
      for (int x = 0; x < nv12->width; x++) {
        nv12->data[0][y * nv12->linesize[0] + x] = yuv420p->data[0][y * yuv420p->linesize[0] + x];
      }
    }
  }
  // Cb and Cr
  for (int y = 0; y < nv12->height / 2; y++) {
    for (int x = 0; x < nv12->width / 2; x++) {
      nv12->data[1][y * nv12->linesize[1] + 2 * x]     = yuv420p->data[1][y * yuv420p->linesize[1] + x];
      nv12->data[1][y * nv12->linesize[1] + 2 * x + 1] = yuv420p->data[2][y * yuv420p->linesize[2] + x];
    }
  }

  return nv12;
}
}  // namespace
void yuv420p_to_nv12(av_frame& yuv420p) {
  const auto format = static_cast<enum AVPixelFormat>(yuv420p.get()->format);

  if (yuv420p.get()->format != AV_PIX_FMT_YUV420P) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }
  av_frame nv12;
  nv12.get()->format = AV_PIX_FMT_NV12;
  nv12.get()->width  = yuv420p.get()->width;
  nv12.get()->height = yuv420p.get()->height;
  nv12.allocate_buffer(32);
  nv12.make_writable();

  // Y
  if (yuv420p.get()->linesize[0] == yuv420p.get()->width) {
    memcpy(nv12.get()->data[0], yuv420p.get()->data[0], yuv420p.get()->height * yuv420p.get()->linesize[0]);
  } else {
    for (int y = 0; y < nv12.get()->height; y++) {
      for (int x = 0; x < nv12.get()->width; x++) {
        nv12.get()->data[0][y * nv12.get()->linesize[0] + x] =
            yuv420p.get()->data[0][y * yuv420p.get()->linesize[0] + x];
      }
    }
  }
  // Cb and Cr
  for (int y = 0; y < nv12.get()->height / 2; y++) {
    for (int x = 0; x < nv12.get()->width / 2; x++) {
      nv12.get()->data[1][y * nv12.get()->linesize[1] + 2 * x] =
          yuv420p.get()->data[1][y * yuv420p.get()->linesize[1] + x];
      nv12.get()->data[1][y * nv12.get()->linesize[1] + 2 * x + 1] =
          yuv420p.get()->data[2][y * yuv420p.get()->linesize[2] + x];
    }
  }

  yuv420p = std::move(nv12);
}
