/*
 * Copyright (c) 2022, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AOM_AV1_DUCKY_ENCODE_H_
#define AOM_AV1_DUCKY_ENCODE_H_

#include <string>
#include <vector>
#include <memory>

#include "aom/aom_encoder.h"
#include "av1/encoder/firstpass.h"
#include "av1/ratectrl_qmode_interface.h"

namespace aom {
struct VideoInfo {
  int frame_width;
  int frame_height;
  aom_rational_t frame_rate;
  aom_img_fmt_t img_fmt;
  int frame_count;
  std::string file_path;
};

struct EncodeFrameResult {
  std::vector<uint8_t> bitstream_buf;
  // TODO(angiebird): update global_coding_idx and global_order_idx properly.
  int global_coding_idx;
  int global_order_idx;
  int q_index;
  int rdmult;
  int rate;
  int64_t dist;
  double psnr;
};

enum class EncodeFrameMode {
  kNone,          // Let native AV1 determine q index and rdmult
  kQindex,        // DuckyEncode determines q index and AV1 determines rdmult
  kQindexRdmult,  // DuckyEncode determines q index and rdmult
  kGopInfo        // DuckyEncode determines the gop structure
};

struct EncodeFrameDecision {
  EncodeFrameMode mode;
  FrameEncodeParameters parameters;
};

using GopEncodeInfoList = std::vector<GopEncodeInfo>;

// DuckyEncode is an experimental encoder c++ interface for two-pass mode.
// This object can be used to do zero or more encode passes, where each encode
// pass consists of:
// - StartEncode()
// - Zero or more calls to EncodeFrame()
// - EndEncode()
// Encode passes may not overlap, and any other sequence of these calls is
// invalid.
class DuckyEncode {
 public:
  explicit DuckyEncode(const VideoInfo &video_info);
  ~DuckyEncode();
  std::vector<FIRSTPASS_STATS> ComputeFirstPassStats();
  void StartEncode(const std::vector<FIRSTPASS_STATS> &stats_list);
  TplGopStats ObtainTplStats(const GopStruct gop_struct);
  std::vector<TplGopStats> ComputeTplStats(const GopStructList &gop_list);
  void EncodeVideo(const GopStructList &gop_list,
                   const GopEncodeInfoList &gop_encode_info_list);
  EncodeFrameResult EncodeFrame(const EncodeFrameDecision &decision);
  void EndEncode();

 private:
  class EncodeImpl;
  std::unique_ptr<EncodeImpl> impl_ptr_;
};
}  // namespace aom

#endif  // AOM_AV1_DUCKY_ENCODE_H_