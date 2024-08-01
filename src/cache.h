#ifndef CACHE_H
#define CACHE_H

#include <atomic>
#include <mutex>
#include <thread>

#include "gop_segment.h"

class cache {
 public:
  cache();
  ~cache();

 private:
  std::vector<gop_segment> gop_segments_{};  ///< 缓存的数据实体,使用环形队列进行缓存,在构造时大小已经确定
  int buffer_size_   = 0;                    // 缓存的大小,一个gop粗略按照0.5s来进行处理
  bool buffer_empty_ = true;                 // 只有初始化的时候为空,添加数据后不再为空
  int buffer_header_ = 0;  // 环形队列的开头,当buffer_empty_是false的时候,header表示可以读取数据的地方
  int buffer_tail_ = 0;  // 环形队列的结尾,是可以写入数据的地方;写入后,需要检查,当tail和header相等时,header需要向后移动
  // 向后移动
  void buffer_header_move() {
    if (buffer_header_ == buffer_size_ - 1) {
      buffer_header_ = 0;
    } else {
      buffer_header_++;
    }
  }
  // 向后移动
  void buffer_tail_move() {
    if (buffer_tail_ == buffer_size_ - 1) {
      buffer_tail_ = 0;
    } else {
      buffer_tail_++;
    }
  }

  std::mutex buffer_mutex_{};              ///< buffer锁
  std::thread buffering_thread_{};         ///< 缓存线程
  std::atomic<bool> is_buffering_{false};  ///< 缓存标识

 public:
  void set_cache_time(const int cache_time_with_seconds) {
    buffer_size_   = cache_time_with_seconds * 2;
    buffer_header_ = buffer_size_ - 1;
    gop_segments_  = std::vector<gop_segment>(buffer_size_);
  }

  /**
   * @brief 该接口会启动一个缓存线程对rtsp流数据进行缓存
   * @param rtsp_url 需要缓存的rtsp流地址
   * @exception 当正在缓存时的时候调用接口会抛出异常
   */
  void start_buffering(std::string rtsp_url);

  /**
   * @brief 停止缓存的接口,会让缓存线程停止运行,并且可以进行下一次缓存,每一次缓存的数据相互独立
   */
  void stop_buffering();

  auto get_buffer() -> std::vector<gop_segment> {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    std::vector<gop_segment> ret{};
    if (buffer_empty_) {
      return {};
    }

    for (int i = buffer_header_; i != buffer_tail_;) {
      ret.emplace_back(gop_segments_[i]);
      if (i == buffer_size_ - 1) {
        i = 0;
      } else {
        i++;
      }
    }

    return ret;
  }

 private:
  /**
   * @brief 实际缓存数据的线程
   * @param rtsp_url 需要缓存的rtsp流地址
   */
  void buffer_rtsp_stream(std::string rtsp_url);
};

#endif  // CACHE_H
