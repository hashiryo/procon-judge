#pragma once
// 診断用の提出 diag_nopf_p0 から p5 が共有する中身。先読みを抜いた libsais (libsais_nopf.hpp と同じ) の 1 段目 (libsais_main_8u) を、
// _diag_split_neo.hpp と同じ 5 つの段に分け、計測区間の run() では DIAG_PHASE の段だけを回す。それより前の段は構築子で、後の段は
// answer() で回す。DIAG_PHASE が 0 なら run() で 5 つの段をすべて回し、出力の配列の確保と 0 埋めだけを構築子に出す。libsais.c には
// 手を入れず、libsais_main_8u の中身 (OpenMP を使わない 1 スレッド、fs = 0、BWT ではない形) を段ごとに写して、その static 関数を呼ぶ。
// 1: 数えて LMS を集める走査とバケットの境目。作業用の表の確保も含む。
// 2: LMS の部分文字列を並べて名前を付け、縮めた文字列を作る段 (radix sort、partial sorting、renumber_and_gather)。
// 3: 再帰 (libsais_main_32s_entry)。
// 4: LMS の位置を集め直し、縮めた問題の接尾辞配列を LMS の位置に直す段 (gather_lms_suffixes と reconstruct)。
// 5: 最後の induced sorting (LMS をバケットの末尾へ移す place_lms_suffixes_interval を含む)。
// 調べ終えたら消す。接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <cstdint>
#include <vector>
#include "pj.hpp"
#define __builtin_prefetch(address, ...) ((void)(address))
#include "_libsais/libsais.c"
#undef __builtin_prefetch

namespace diag_split_libsais {
struct State {
  const uint8_t *T = nullptr;
  sa_sint_t *SA = nullptr, *buckets = nullptr;
  sa_sint_t n = 0, m = 0, k = 0, names = 0;
  void phase(int p) {
    const sa_sint_t flags = LIBSAIS_FLAGS_NONE, threads = 1, fs = 0;
    LIBSAIS_THREAD_STATE *ts = NULL;
    if (n < 2) {
      if (p == 5 && n == 1) SA[0] = 0;
      return;
    }
    if (p == 1) {
      buckets = (sa_sint_t *)libsais_alloc_aligned((size_t)8 * ALPHABET_SIZE * sizeof(sa_sint_t), 4096);
      m = libsais_count_and_gather_lms_suffixes_8u_omp(T, SA, n, buckets, threads, ts);
      k = libsais_initialize_buckets_start_and_end_8u(buckets, NULL);
    } else if (p == 2) {
      if (m == 0) return;
      const sa_sint_t first_lms_suffix = SA[n - m];
      const sa_sint_t left_suffixes_count = libsais_initialize_buckets_for_lms_suffixes_radix_sort_8u(T, buckets, first_lms_suffix);
      libsais_radix_sort_lms_suffixes_8u_omp(T, SA, n, m, flags, buckets, threads, ts);
      libsais_initialize_buckets_for_partial_sorting_8u(T, buckets, first_lms_suffix, left_suffixes_count);
      libsais_induce_partial_order_8u_omp(T, SA, n, k, flags, buckets, first_lms_suffix, left_suffixes_count, threads, ts);
      names = libsais_renumber_and_gather_lms_suffixes_omp(SA, n, m, fs, threads, ts);
    } else if (p == 3) {
      if (m == 0 || names >= m) return;
      libsais_main_32s_entry(SA + n + fs - m, SA, m, names, fs + n - 2 * m, threads, ts);
    } else if (p == 4) {
      if (m == 0 || names >= m) return;
      libsais_gather_lms_suffixes_8u_omp(T, SA, n, threads, ts);
      libsais_reconstruct_lms_suffixes_omp(SA, n, m, threads);
    } else if (p == 5) {
      if (m > 0)
        libsais_place_lms_suffixes_interval_8u(SA, n, m, flags, buckets);
      else
        memset(SA, 0, (size_t)n * sizeof(sa_sint_t));
      libsais_induce_final_order_8u_omp(T, SA, n, k, flags, 0, NULL, buckets, threads, ts);
      libsais_free_aligned(buckets);
    }
  }
};
}  // namespace diag_split_libsais

struct Solver {
  string s;
  vector<int> sa;
  mutable diag_split_libsais::State st;

  explicit Solver(const string &s) : s(s), sa(s.size()) {
    st.T = reinterpret_cast<const uint8_t *>(this->s.data()), st.n = sa_sint_t(this->s.size()), st.SA = sa.data();
    for (int p = 1; p < DIAG_PHASE; ++p) st.phase(p);
  }

  void run() {
    if (DIAG_PHASE == 0)
      for (int p = 1; p <= 5; ++p) st.phase(p);
    else
      st.phase(DIAG_PHASE);
  }

  const vector<int> &answer() const {
    if (DIAG_PHASE != 0)
      for (int p = DIAG_PHASE + 1; p <= 5; ++p) st.phase(p);
    return sa;
  }
};
