#include "api.h"

#include <fstream>
#include <gsl/util>

#include "encode_support.h"

void api::start_cache(const std::string& rtsp_url, int cache_time_with_seconds) {
  // 开始缓存
  stream_.set_cache_time(cache_time_with_seconds);
  stream_.start_buffering(rtsp_url);
  // 从rtsp流中获取信息,为解码和编码做准备
  rtsp_stream_info_.set_format_from(rtsp_url);
}
void api::save_evidence(const std::vector<save_evidence_info>& boxs_info, const std::string& file_path) {
  if (boxs_info.empty()) {
    return;
  }
  // 创建编码器
  decoding<AVCodecID> decoder(rtsp_stream_info_.get_video_type());
  decoder.add_video_param(rtsp_stream_info_.get_video_params());
  decoder.create_decoder();
  // 获取缓存数据
  auto bufffer = stream_.get_buffer();
  // pts偏移量计算
  int offset = 0;
  {
    auto first_pts                  = boxs_info[0].pts;
    int first_bigger_than_first_pts = 0;
    for (auto gop : bufffer) {
      for (auto packet_info : gop.packet) {
        if (packet_info.dts == first_pts) {
          offset = 0;
          break;
        } else if (packet_info.dts > first_pts) {  // 找到了第一个大于画框组的帧的pts就可以开始计算offset了
          first_bigger_than_first_pts = packet_info.dts;
          offset                      = first_bigger_than_first_pts - first_pts;
          break;
        }
      }
      if (offset != 0) {
        break;
      }
    }
  }
  // 在解码前先定位需要处理的gop是那几组
  int start_gop_num = 0;
  int end_top_num   = 0;
  int frame_count   = 0;
  int frame_start   = 0;
  int frame_end     = 0;
  {
    const int draw_start_pts = boxs_info[0].pts + offset;
    const int draw_end_pts   = boxs_info.back().pts + offset;
    for (int i = 0; i < bufffer.size(); ++i) {
      if (frame_start==0)frame_count = 0;
      for (const auto& pkg : bufffer[i].packet) {
        frame_count++;
        if (draw_start_pts == pkg.dts) {
          start_gop_num = i;
          frame_start   = frame_count;
        }
        if (draw_end_pts == pkg.dts) {
          end_top_num = i;
          frame_end   = frame_count;
          break;
        }
      }
      if (end_top_num != 0) {
        break;
      }
    }
  }
  // 对指定的组进行解码
  std::vector<AVFrame*> frames{};
  auto frames_release = gsl::finally([&]() {
    for (int i = 0; i < frames.size(); i++) {
      av_frame_free(&frames[i]);
    }
  });
  for (int i = start_gop_num; i <= end_top_num; ++i) {
    // 组装 std::vector<AVPacket*>
    std::vector<AVPacket*> packets;
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    for (const auto& packet_info : bufffer[i].packet) {
      AVPacket* temp;
      temp       = av_packet_alloc();
      temp->data = static_cast<uint8_t*>(av_malloc(packet_info.pkg_data.size()));

      memcpy(temp->data, packet_info.pkg_data.data(), packet_info.pkg_data.size());
      temp->size = static_cast<int>(packet_info.pkg_data.size());
      temp->pts  = packet_info.pts;
      temp->dts  = packet_info.dts;
      packets.emplace_back(temp);
    }
    // 解码
    auto temp = decoder.independent_decoder(bufffer[i].extradata, packets);
    for (auto frame : temp) {
      frames.emplace_back(frame);
    }
  }
  // 绘制
  for (int i = 0; i < frames.size(); i++) {
    if (i >= frame_start && i <= frame_end) {
      static int box_index = 0;
      draw_detection_boxes(frames[i], boxs_info[box_index++].boxs);
    }
  }
  // 最后进行编码和保存
  {
    encoding<AVCodecID> encoder(AV_CODEC_ID_H264);
    auto codec_ctx = rtsp_stream_info_.get_stream_ctx()->streams[rtsp_stream_info_.get_video_stream_index()]->codecpar;
    std::vector<uint8_t> extradata(codec_ctx->extradata, codec_ctx->extradata + codec_ctx->extradata_size);
    auto packets         = encoder.get_encode_packets(frames, extradata);
    auto packets_release = gsl::finally([&]() {
      for (auto& packet : packets) {
        av_packet_free(&packet);
      }
    });
    // 将编码数据写入文件
    {
      std::ofstream file(file_path, std::ios::out | std::ios::binary);
      for (auto packet : packets) {
        file.write((char*)packet->data, packet->size);
      }
      file.close();
    }
  }
}
