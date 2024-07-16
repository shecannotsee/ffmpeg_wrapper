//
// Created by shecannotsee on 24-7-12.
//

#ifndef RTSP_BUFFER_H
#define RTSP_BUFFER_H

#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#include "gop_segment.h"
#include "safe_queue.h"

class rtsp_buffer {
 public:
  rtsp_buffer();
  ~rtsp_buffer();

 private:
  std::vector<gop_segment> gop_segments_{};  ///< 缓存的数据实体

  std::mutex buffer_mutex_{};              ///< buffer锁
  std::thread buffering_thread_{};         ///< 缓存线程
  std::atomic<bool> is_buffering_{false};  ///< 缓存标识

 public:
  /**
   * @brief 该接口会启动一个缓存线程对rtsp流数据进行缓存
   * @param rtsp_url 需要缓存的rtsp流地址
   * @exception 当正在缓存时的时候调用接口会抛出异常
   */
  void start_buffering(std::string rtsp_url);

  /**
   * @brief 停止缓存的接口,会让缓存线程停止运行,并且可以进行下一次缓存
   */
  void stop_buffering();

  auto get_buffer() {
    return gop_segments_;
  }

 private:
  /**
   * @brief 实际缓存数据的线程
   * @param rtsp_url 需要缓存的rtsp流地址
   */
  void buffer_rtsp_stream(std::string rtsp_url);
};

#endif  // RTSP_BUFFER_H
