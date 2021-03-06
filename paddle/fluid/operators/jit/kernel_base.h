/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#pragma once
#include <cstdint>
#include "paddle/fluid/operators/jit/macro.h"
#include "paddle/fluid/platform/macros.h"

namespace paddle {
namespace operators {
namespace jit {

typedef enum {
  kNone = 0,
  // sort by alphabet
  kCRFDecoding = 1,
  kEmbSeqPool = 2,
  kGRUH1,
  kGRUHtPart1,
  kGRUHtPart2,
  kHSum,  // horizontal max
  kHMax,  // horizontal sum
  kLSTMCtHt,
  kLSTMC1H1,
  kLayerNorm,
  kMatMul,
  kNCHW16CMulNC,
  kSeqPool,
  kSoftmax,
  kVAdd,
  kVAddBias,
  kVAddRelu,
  kVExp,
  kVIdentity,
  kVMul,
  kVRelu,
  kVScal,
  kVSigmoid,
  kVSquare,
  kVSub,
  kVTanh,
} KernelType;

typedef enum {
  kNonePoolType = 0,
  kSum = 1,
  kAvg,
  kSqrt,
} SeqPoolType;

template <typename T>
struct XYZNTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(const T*, const T*, T*, int);
};

template <typename T>
struct AXYNTuples : public XYZNTuples<T> {};

template <typename T>
struct XYNTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(const T*, T*, int);
};

// x, return and int
template <typename T>
struct XRNTuples : public XYNTuples<T> {};

typedef struct {
  void* gates;  // gates: x_ch, x_ih, x_fh, x_oh
  const void* ct_1;
  void* ct;
  void* ht;
  /* weight_peephole and checked data are only used in peephole*/
  const void* wp{nullptr};  //  W_ic, W_fc, W_oc
  void* checked{nullptr};   // size: 2 * d
} lstm_t;

typedef struct {
  void* gates;  // gates: {x_update, x_reset; x_state}
  const void* ht_1;
  void* ht;
} gru_t;

struct rnn_attr_s {
  int d;
  KernelType act_gate, act_cand;
  rnn_attr_s() = default;
  explicit rnn_attr_s(int _d, KernelType _act_gate, KernelType _act_cand)
      : d(_d), act_gate(_act_gate), act_cand(_act_cand) {}
};

struct lstm_attr_s : public rnn_attr_s {
  bool use_peephole;
  KernelType act_cell;
  lstm_attr_s() = default;
  explicit lstm_attr_s(int _d, KernelType _act_gate, KernelType _act_cand,
                       KernelType _act_cell, bool _use_peephole = false)
      : rnn_attr_s(_d, _act_gate, _act_cand),
        use_peephole(_use_peephole),
        act_cell(_act_cell) {}
};

typedef struct rnn_attr_s gru_attr_t;
typedef struct lstm_attr_s lstm_attr_t;

template <typename T>
struct LSTMTuples {
  typedef T data_type;
  typedef lstm_attr_t attr_type;
  typedef void (*func_type)(lstm_t*, const lstm_attr_t*);
};

template <typename T>
struct GRUTuples {
  typedef T data_type;
  typedef gru_attr_t attr_type;
  typedef void (*func_type)(gru_t*, const gru_attr_t*);
};

typedef struct seq_pool_attr_s {
  int h, w;  // h should always be the first one
  SeqPoolType type;
  seq_pool_attr_s() = default;
  explicit seq_pool_attr_s(int width, SeqPoolType pool_type, int height = 1)
      : h(height), w(width), type(pool_type) {}
} seq_pool_attr_t;

template <typename T>
struct SeqPoolTuples {
  typedef T data_type;
  typedef seq_pool_attr_t attr_type;
  typedef void (*func_type)(const T*, T*, const seq_pool_attr_t*);
};

typedef struct emb_seq_pool_attr_s {
  int64_t table_height, table_width;
  int64_t index_height, index_width;
  int64_t out_width;
  SeqPoolType pool_type;
  emb_seq_pool_attr_s() = default;
  explicit emb_seq_pool_attr_s(int64_t tbl_height, int64_t tbl_width,
                               int64_t idx_height, int64_t idx_width,
                               int64_t output_width,
                               SeqPoolType seqpool_type = SeqPoolType::kSum)
      : table_height(tbl_height),
        table_width(tbl_width),
        index_height(idx_height),
        index_width(idx_width),
        out_width(output_width),
        pool_type(seqpool_type) {}
} emb_seq_pool_attr_t;

template <typename T>
struct EmbSeqPoolTuples {
  typedef T data_type;
  typedef emb_seq_pool_attr_t attr_type;
  typedef void (*func_type)(const T*, const int64_t*, T*,
                            const emb_seq_pool_attr_t*);
};

typedef struct matmul_attr_s {
  int m, n, k;
  void* packed_weight{nullptr};
  matmul_attr_s() = default;
  explicit matmul_attr_s(int m_, int n_, int k_, void* packed_weight_ = nullptr)
      : m(m_), n(n_), k(k_), packed_weight(packed_weight_) {}
} matmul_attr_t;

template <typename T>
struct MatMulTuples {
  typedef T data_type;
  typedef matmul_attr_t attr_type;
  typedef void (*func_type)(const T*, const T*, T*, const matmul_attr_t*);
};

template <typename T>
struct CRFDecodingTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(const int, const T*, const T*, T*, int*, int);
};

template <typename T>
struct LayerNormTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(T*, T*, T*, T*, const T*, const T*, int,
                            const float, int);
};

template <typename T>
struct SoftmaxTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(const T*, T*, int, int);
};

// nChw16c = nChw16c .* NC
template <typename T>
struct NCHW16CMulNCTuples {
  typedef T data_type;
  typedef int attr_type;
  typedef void (*func_type)(const T*, const T*, T*, int, int);
};

// Just for adding to kernel pool without template
class Kernel {
 public:
  Kernel() = default;
  virtual ~Kernel() = default;
  DISABLE_COPY_AND_ASSIGN(Kernel);
};

template <typename KernelTuples>
class KernelMore : public Kernel {
 public:
  using T = typename KernelTuples::data_type;
  using Func = typename KernelTuples::func_type;
  using Attr = typename KernelTuples::attr_type;
  virtual Func GetFunc() const { return func; }
  virtual bool UseMe(const Attr& attr) const = 0;
  virtual const char* ImplType() const = 0;

 protected:
  Func func{nullptr};
};

template <typename KernelTuples>
class ReferKernel : public KernelMore<KernelTuples> {
 public:
  // Refer code can always be used
  bool UseMe(const typename KernelTuples::attr_type& attr) const override {
    return true;
  }
  const char* ImplType() const override { return "Refer"; }
};

}  // namespace jit
}  // namespace operators
}  // namespace paddle
