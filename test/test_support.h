#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace test_support {

/**
 * @brief 该函数用来检查目录下是否存在某目录
 * @param dir_path example:"/path/to/your/directory"
 * @param dir_name example:"target_folder"
 * @return true表示存在，false表示不存在
 */
static bool is_directory_exists(const std::string& dir_path, const std::string& dir_name) {
  DIR* dir = opendir(dir_path.c_str());
  if (dir == nullptr) {
    return false;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_type == DT_DIR && entry->d_name == dir_name) {
      closedir(dir);
      return true;
    }
  }

  closedir(dir);
  return false;
}

/**
 * @brief 该函数用来检查目录下是否存在某目录
 * @param full_path 完整路径
 * @return true表示存在，false表示不存在
 */
static bool is_directory_exists(const std::string& full_path) {
  struct stat info {};
  if (stat(full_path.c_str(), &info) != 0) {
    // Cannot access the path (file/directory does not exist)
    return false;
  } else if (info.st_mode & S_IFDIR) {
    // The path exists and it is a directory
    return true;
  } else {
    // The path exists but it is not a directory
    return false;
  }
}

/**
 * @brief 该函数用来创建目录，创建时会检查目录是否存在
 * @param dir_path 目录路径
 * @throw 创建失败
 */
static void create_dir(const std::string& dir_path) {
  if (is_directory_exists(dir_path)) {
    return;
  }
  // Mode 0755 gives read, write, and execute permissions to the owner,
  // and read and execute permissions to group and others
  mode_t mode = 0755;

  // Create the directory
  int result = mkdir(dir_path.c_str(), mode);

  if (result != 0) {
    throw std::runtime_error(std::string("Failed to create directory: " + dir_path));
  }
}

/**
 * @brief 删除指定目录
 * @param dir_path 要删除的目录路径
 * @throw 删除失败
 */
static void remove_directory(const std::string& dir_path) {
  if (rmdir(dir_path.c_str()) != 0) {
    throw std::runtime_error(std::string("Failed to delete directory " + dir_path));
  }
}

/**
 * @brief 自定义数据写入文件
 * @param fileName 写入文件名
 * @param data 数据
 * @param size 数据长度
 * @param append 尾部添加或者是清空文件写入
 */
static void write_to_file(const std::string& fileName, const char* data, size_t size, bool append = false) {
  std::ofstream file;
  if (append) {
    file.open(fileName, std::ios::out | std::ios::app);  // 追加模式
  } else {
    file.open(fileName, std::ios::out | std::ios::trunc);  // 清空模式
  }

  if (file.is_open()) {
    file.write(data, size);
    file.close();
  } else {
    std::cerr << "无法打开文件进行写入: " << fileName << std::endl;
  }
}

/**
 * @brief 获取当前毫秒时间戳
 * @return 以毫秒为单位的时间戳
 */
static long long get_current_milliseconds_timestamp() {
  auto now      = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
  return duration.count();
}

/**
 * @brief 将以毫秒为单位的时间戳转换成日期字符串
 * @param milliseconds
 * @return format: 2024-05-30 11:12:52.233
 */
static std::string convert_milliseconds_to_datetime_string(long long milliseconds) {
  // Convert milliseconds to time_t (seconds since epoch)
  std::time_t seconds = milliseconds / 1000;

  // Convert time_t to tm structure for local time
  std::tm* tm_time = std::localtime(&seconds);

  // Create a stringstream to format the date and time
  std::ostringstream oss;
  oss << std::put_time(tm_time, "%Y-%m-%d %H:%M:%S");

  // Append the milliseconds part
  oss << "." << std::setfill('0') << std::setw(3) << (milliseconds % 1000);

  return oss.str();
}

/**
 * @brief 该类主要用来记录事件消耗的时间
 */
class timer {
 public:
  /**
   * @brief 构造函数，自动记录一次开始时间
   */
  timer() {
    start_time = std::chrono::high_resolution_clock::now();
  }

  /**
   * @brief 开始记录时间点
   */
  void start() {
    start_time = std::chrono::high_resolution_clock::now();
  }

  /**
   * @brief 结束记录时间点
   */
  void end() {
    end_time = std::chrono::high_resolution_clock::now();
  }

  /**
   * @brief 该接口用来统计记录开始和结束的时候的用时
   * @tparam Duration
   * std::chrono::seconds 秒;
   * std::chrono::milliseconds 毫秒;
   * std::chrono::microseconds 微秒;
   * @return 输出消耗的时间
   */
  template <typename Duration>
  long long get_elapsed() const {
    Duration elapsed = std::chrono::duration_cast<Duration>(end_time - start_time);
    return elapsed.count();
  }

 private:
  std::chrono::high_resolution_clock::time_point start_time;  ///< 开始时间戳
  std::chrono::high_resolution_clock::time_point end_time;    ///< 结束时间戳
};

/**
 * @brief 从文件中读取二进制数据
 * @param file_path 读取数据的文件路径
 * @return 读取的二进制数据流
 */
static std::vector<char> read_binary_file(const std::string& file_path) {
  // 打开文件，二进制模式和输入模式
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error(std::string("Failed to open file: " + file_path));
  }

  // 获取文件大小
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  // 创建缓冲区
  std::vector<char> buffer(size);
  if (file.read(buffer.data(), size)) {
    return buffer;
  } else {
    throw std::runtime_error(std::string("Failed to read file: " + file_path));
  }
}

}  // namespace test_support

#endif  // TEST_SUPPORT_H
