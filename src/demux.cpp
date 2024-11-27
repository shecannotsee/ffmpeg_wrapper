#include "demux.h"

#include <stdexcept>

demux::demux() noexcept : fmt_ctx_(nullptr) {
}

demux::~demux() {
  if (fmt_ctx_ != nullptr) {
    avformat_close_input(&fmt_ctx_);
    fmt_ctx_ = nullptr;
  }
}

void demux::open(const std::string &url, const demux::key_value_list &params) {
  url_ = url;

  // Open the input file and allocate the format context
  AVDictionary *opts = nullptr;
  for (const auto &param : params) {
    auto [key, value, flags] = param;
    av_dict_set(&opts, key.c_str(), value.c_str(), flags);
  }
  auto ret = avformat_open_input(&fmt_ctx_, url_.c_str(), nullptr, &opts);
  if (ret < 0) {
    char err_buf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(ret, err_buf, sizeof(err_buf));
    const std::string err_msg = "Could not open " + url_ + ": " + err_buf;
    throw std::runtime_error(err_msg);
  }
  if (opts) {
    av_dict_free(&opts);
    opts = nullptr;
  }

  // Retrieve stream information
  ret = avformat_find_stream_info(fmt_ctx_, nullptr);
  if (ret < 0) {
    const std::string err_msg = "Could not find stream information for " + url_;
    avformat_close_input(&fmt_ctx_);
    throw std::runtime_error(err_msg);
  }

  // Method 1: Get stream information
  bool find_video = false;
  bool find_audio = false;
  for (unsigned int i = 0; i < fmt_ctx_->nb_streams; i++) {
    if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_.stream_index = static_cast<int>(i);
      find_video          = true;
    } else if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audio_.stream_index = static_cast<int>(i);
      find_audio          = true;
    }
    if (find_video && find_audio) {
      return;
    }
  }

  // Method 2: Use `av_find_best_stream` to find video/audio stream
  // video_.stream_index = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  // audio_.stream_index = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

  if (video_.stream_index < 0 || audio_.stream_index < 0) {
    std::string err_msg;
    if (video_.stream_index < 0) {
      err_msg +=
          "Could not find " + std::string(av_get_media_type_string(AVMEDIA_TYPE_VIDEO)) + " stream in " + url_;
    }
    if (audio_.stream_index < 0) {
      err_msg += "Could not find " + std::string(av_get_media_type_string(AVMEDIA_TYPE_AUDIO)) + " stream in " + url_;
    }

    if (video_.stream_index < 0 && audio_.stream_index < 0) {
      throw std::runtime_error(err_msg);
    }
  }
}
