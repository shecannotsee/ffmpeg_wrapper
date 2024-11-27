#include "image_support.h"

extern "C" {
#include <libavutil/imgutils.h>
}

#include "av_codec_context_support.h"
#include "encode.h"

void pgm_save(av_frame& frame, const std::string& file_name) {
  const auto buf  = frame.get()->data[0];
  const int wrap  = frame.get()->linesize[0];
  const int xsize = frame.get()->width;
  const int ysize = frame.get()->height;

  FILE* f = fopen(file_name.c_str(), "w");
  fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);  // 格式化写入内容
  for (int i = 0; i < ysize; i++) {
    fwrite(buf + i * wrap, 1, xsize, f);
  }
  fclose(f);
}

void jpeg_save(av_frame& frame, const std::string& file_name) {
  if (const auto format = static_cast<enum AVPixelFormat>(frame.get()->format);
      format != AV_PIX_FMT_YUV420P      // 标准的 YUV 420 格式
      && format != AV_PIX_FMT_YUVJ420P  // 常用的 JPEG 输入格式
      && format != AV_PIX_FMT_YUVJ422P  // 适用于部分 JPEG 编码的格式
      && format != AV_PIX_FMT_YUVJ444P  // 高质量的 JPEG 编码格式
      && format != AV_PIX_FMT_RGB24     // 可以直接编码为 JPEG 格式
      && format != AV_PIX_FMT_RGBA      // 带透明通道的 RGB 格式，也可编码为 JPEG，但透明通道会被丢弃
  ) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }
  encode encoder(AV_CODEC_ID_MJPEG);
  av_codec_context_support ctx;
  const auto jpeg_ctx = ctx.get_jpeg_encode(frame.get()->width, frame.get()->height);
  encoder.create_encoder(jpeg_ctx);

  auto jpeg_list = encoder.encoding(frame);

  for (auto& jpeg : jpeg_list) {
    FILE* f = fopen(file_name.c_str(), "w");
    if (!f) {
      throw std::runtime_error("Unable to open file for writing: " + file_name);
    }
    fwrite(jpeg.get()->data, 1, jpeg.get()->size, f);
    fclose(f);
  }
}
