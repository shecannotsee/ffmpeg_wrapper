#include "stream_support.h"

stream_info::stream_info() : video_stream_index_(-1), audio_stream_index_(-1) {
  format_ctx_ = avformat_alloc_context();
}

stream_info::~stream_info() {
  avformat_close_input(&format_ctx_);
}

void stream_info::set_format_from(const std::string& url, const std::vector<stream_param>& params) noexcept {
  url_ = url;

  // 使用参数打开地址
  AVDictionary* opts = nullptr;
  for (const auto& param : params) {
    av_dict_set(&opts, param.key.c_str(), param.value.c_str(), param.flag);
  }
  auto ret = avformat_open_input(&format_ctx_, url_.c_str(), nullptr, &opts);
  if (ret != 0) {
    throw std::runtime_error("Could not open url");
  }
  if (opts) {
    av_dict_free(&opts);
  }

  // 在流中查找信息
  ret = avformat_find_stream_info(format_ctx_, nullptr);
  if (ret < 0) {
    throw std::runtime_error("Could not find stream information.");
  }

  // get video info
  for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
    if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index_ = i;
    }
  }

  // get audio info
  for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
    if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audio_stream_index_ = i;
    }
  }
}

AVCodecParameters* stream_info::get_video_params() const {
  if (video_stream_index_ != -1) {
    return format_ctx_->streams[video_stream_index_]->codecpar;
  }
  throw std::runtime_error("Get video params failed");
}

AVCodecID stream_info::get_video_type() const {
  if (video_stream_index_ != -1) {
    return get_video_params()->codec_id;
  }
  throw std::runtime_error("Get video params failed");
}

AVFormatContext* stream_info::get_stream_ctx() const {
  return format_ctx_;
}

int stream_info::get_video_stream_index() const {
  return video_stream_index_;
}

void stream_info::dump() const {
  av_dump_format(format_ctx_, 0, url_.c_str(), 0);
}
