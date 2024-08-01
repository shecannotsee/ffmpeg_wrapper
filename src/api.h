#ifndef API_H
#define API_H

#include <string>
#include <vector>

#include "cache.h"
#include "decode_support.h"
#include "draw_on_the_picture.h"

struct save_evidence_info {
  int pts;
  std::vector<detection> boxs;
};

class api {
  cache stream_;

  pre_decoding rtsp_stream_info_;


 public:
  api() = default;

 public:
  void start_cache(const std::string& rtsp_url, int cache_time_with_seconds);

  void save_evidence(const std::vector<save_evidence_info>& boxs_info,const std::string& file_path);
};

#endif  // API_H
