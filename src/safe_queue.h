#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class safe_queue {
 public:
  safe_queue()  = default;
  ~safe_queue() = default;

  // 禁用复制构造和复制赋值
  safe_queue(const safe_queue& other)            = delete;
  safe_queue& operator=(const safe_queue& other) = delete;

 private:
  mutable std::mutex mutex_;
  std::queue<T> queue_;
  std::condition_variable cond_var_;

  // 添加数据到队列
  void push(T value) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(value));
    }
    cond_var_.notify_one();  // Ensure notify is called outside the lock
  }

  // 从队列取出数据
  bool pop(T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] { return !queue_.empty(); });
    if (queue_.empty()) {
      return false;  // This check is generally not needed but added for safety
    }
    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  // 尝试从队列取出数据（非阻塞）
  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  // 检查队列是否为空
  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  // 获取队列大小
  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }
};

#endif //SAFE_QUEUE_H
