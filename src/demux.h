#ifndef FFMPEG_WRAPPER_DEMUX_H
#define FFMPEG_WRAPPER_DEMUX_H

extern "C" {
#include <libavformat/avformat.h>
}

#include <string>
#include <tuple>
#include <vector>

#include "av_packet.h"

struct stream_param {
  std::string key;
  std::string value;
  int flag;
};

/**
 * @brief A class for handling audio and video demuxing.
 *
 * This class uses FFmpeg to demux audio and video streams. It can open media files and retrieve packets of specified types.
 */
class demux {
  std::string url_;           ///< URL of the media file
  AVFormatContext* fmt_ctx_;  ///< FFmpeg format context
  struct {
    int stream_index{-1};  ///< Stream index
  } video_, audio_;

 public:
  /**
   * @brief Default constructor.
   *
   * Constructs a `demux` object and initializes the logger.
   */
  demux() noexcept;

  /**
   * @brief Destructor.
   *
   * Frees resources associated with the format context.
   */
  ~demux();

 public:
  /**
   * @brief Type alias for a list of key-value pairs.
   *
   * Used to store a list of parameter key-value pairs, e.g., [{k1,v1,0},{...},{kn,vn,0},{...}].
   */
  using key_value_list = std::vector<std::tuple<std::string, std::string, int>>;

  /**
   * @brief Opens a media file.
   *
   * @param url The URL of the media file.
   * @param params Optional parameters to be passed to the `avformat_open_input` function.
   * @throw std::runtime_error If the file cannot be opened or stream information cannot be retrieved.
   */
  void open(const std::string& url, const key_value_list& params = {});

  /**
   * @brief Enum representing the type of media packet.
   */
  enum class type { audio, video, av };

  /**
   * @brief A class to hold both packet type and the packet itself.
   */
  class type_av_packet {
   public:
    type t;         ///< The type of the packet (audio, video, or av)
    av_packet pkt;  ///< The packet itself
  };

  /**
   * @brief Starts receiving a specified number of packets.
   *
   * @tparam t The type of packet (audio, video, or av).
   * @param num_packets The number of packets to receive.
   * @return A vector containing the packets of the specified type.
   * @throw std::runtime_error If an error occurs while reading the stream.
   */
  template <type t>
  [[nodiscard]] auto start_receiving(size_t num_packets = 25) -> std::vector<type_av_packet>;

 public:
  /**
   * @brief Retrieves the codec parameters for a specified stream type.
   *
   * @tparam t The packet type (either `type::video` or `type::audio`).
   * @return A pointer to the `AVCodecParameters` structure for the specified stream.
   * @throw std::runtime_error If the stream index is invalid or out of range.
   *
   * @note This method should be used after successfully opening the media file and setting the video/audio stream index.
   */
  template <type t>
  [[nodiscard]] auto get_codec_parameters() const -> const AVCodecParameters*;

  /**
   * @brief Retrieves the codec ID for a specified stream type.
   *
   * @tparam t The packet type (either `type::video` or `type::audio`).
   * @return The `AVCodecID` of the specified stream.
   * @throw std::runtime_error If the stream index is invalid or out of range.
   *
   * @note This method should be used after successfully opening the media file and setting the video/audio stream index.
   */
  template <type t>
  [[nodiscard]] auto get_codec_id() -> enum AVCodecID;

  /**
   * @brief Retrieves the media stream for a specified type.
   *
   * @tparam t The type of media stream, either `type::video` or `type::audio`.
   * @return A pointer to the corresponding `AVStream`.
   * @throw static_assert If the template argument is neither `type::video` nor `type::audio`.
   */
  template <type t>
  [[nodiscard]] auto get_stream() -> AVStream*;
};

#include "demux_impl.h"

#endif  // FFMPEG_WRAPPER_DEMUX_H
