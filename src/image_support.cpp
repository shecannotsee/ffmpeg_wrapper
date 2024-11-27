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
  fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);  // Write header in PGM format
  for (int i = 0; i < ysize; i++) {
    fwrite(buf + i * wrap, 1, xsize, f);  // Write image data line by line
  }
  fclose(f);
}

void jpeg_save(av_frame& frame, const std::string& file_name) {
  if (const auto format = static_cast<enum AVPixelFormat>(frame.get()->format);
      format != AV_PIX_FMT_YUV420P      // Standard YUV 420 format
      && format != AV_PIX_FMT_YUVJ420P  // Common JPEG input format
      && format != AV_PIX_FMT_YUVJ422P  // Suitable for certain JPEG encodings
      && format != AV_PIX_FMT_YUVJ444P  // High-quality JPEG encoding format
      && format != AV_PIX_FMT_RGB24     // Can be directly encoded into JPEG
      && format != AV_PIX_FMT_RGBA      // RGB format with alpha channel (alpha channel will be discarded)
  ) {
    const std::string err_msg =
        "Unsupported pixel format: " +
        std::string(av_get_pix_fmt_name(format) ? av_get_pix_fmt_name(format) : "Unknown pixel format");
    throw std::runtime_error(err_msg);
  }

  encode encoder(AV_CODEC_ID_MJPEG);  // Create MJPEG encoder
  av_codec_context_support ctx;
  const auto jpeg_ctx = ctx.get_jpeg_encode(frame.get()->width, frame.get()->height);
  encoder.create_encoder(jpeg_ctx);

  auto jpeg_list = encoder.encoding(frame);  // Encode the frame into JPEG packets

  for (auto& jpeg : jpeg_list) {
    FILE* f = fopen(file_name.c_str(), "w");
    if (!f) {
      throw std::runtime_error("Unable to open file for writing: " + file_name);
    }
    fwrite(jpeg.get()->data, 1, jpeg.get()->size, f);  // Write the encoded JPEG data to file
    fclose(f);
  }
}
