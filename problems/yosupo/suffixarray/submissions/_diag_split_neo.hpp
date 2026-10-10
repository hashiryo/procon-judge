#pragma once
// 診断用の提出 diag_neo_p0 から p5 が共有する中身。NeoLibrary の suffix_array (sais_lr13 と同じ) の 1 段目を 5 つの段に分け、計測区間の
// run() では DIAG_PHASE の段だけを回す。それより前の段は構築子で、後の段は answer() で回すので、出力は正しいまま、その段だけの時間が
// 記録に残る (どれも計測区間の外では測られない)。DIAG_PHASE が 0 なら run() で 5 つの段をすべて回し、出力の配列の確保と 0 埋め
// だけを構築子に出す (lib-neo との差が確保とページフォールトの分)。段の分け方は _diag_split_libsais.hpp と揃える。
// 1: 数える走査 (gather_lms) とバケットの境目。作業用の表の確保も含む。
// 2: LMS の部分文字列を並べて名前を付ける段 (expand_lms と sort_lms_lr か sort_lms_wide) と、名前を詰めて縮めた文字列にする段。
// 3: 再帰 (2 段目以降のすべて)。
// 4: LMS の位置を広げ直し、縮めた問題の接尾辞配列を LMS の位置に直す段。
// 5: 最後の induced sorting (形の見積もり、LMS をバケットの末尾へ移す段を含む)。
// 調べ終えたら消す。接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <memory>
#include <string>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"

namespace diag_split_neo {
using namespace suffix_array_internal;
struct State {
  const unsigned char *s = nullptr;
  int n = 0, K = 256, m = 0, names = 0;
  int *sa = nullptr;
  bool wide = false;
  std::vector<int> q, cnt, bkt;
  std::unique_ptr<uint64_t[]> bits;
  void phase(int p) {
    if (n < 2) {
      if (p == 5 && n == 1) sa[0] = 0;
      return;
    }
    if (p == 1) {
      wide = K > n / 32;
      q.assign(wide ? 0 : 4 * K, 0), cnt.assign(K + 1, 0), bkt.resize(K);
      bits.reset(new uint64_t[(n + 63) / 64]);
      m = wide ? gather_lms<false>(s, n, bits.get(), cnt.data()) : gather_lms<true>(s, n, bits.get(), q.data());
      for (int c = 0; c < K; ++c) cnt[c + 1] = wide ? cnt[c + 1] + cnt[c] : cnt[c] + q[4 * c] + q[4 * c + 1] + q[4 * c + 2] + q[4 * c + 3];
    } else if (p == 2) {
      if (m == 0) return;
      if (wide) {
        std::unique_ptr<int[]> lms(new int[m]);
        expand_lms(bits.get(), n, lms.get());
        names = sort_lms_wide(s, n, K, cnt.data(), bkt.data(), lms.get(), m, sa);
      } else {
        expand_lms(bits.get(), n, sa + n - m);
        names = sort_lms_lr(s, n, K, q.data(), sa + n - m, m, sa);
      }
      if (names < m) {
        for (int i = m + (n >> 1) - 1, l = n; i >= m; --i) {
          const int x = sa[i];
          sa[l - 1] = x & POS, l -= x < 0;
        }
      }
    } else if (p == 3) {
      if (m == 0 || names == m) return;
      sa_is<int>(sa + n - m, m, names, sa);
    } else if (p == 4) {
      if (m == 0) return;
      if (names < m) {
        expand_lms(bits.get(), n, sa + n - m);
        for (int k = 0; k < m; ++k) sa[k] = sa[n - m + sa[k]];
      } else {
        for (int k = 0; k < m; ++k) sa[k] &= POS;
      }
    } else if (p == 5) {
      if (m == 0) {
        induce_inplace<false>(s, n, K, cnt.data(), 0, sa, bkt.data());
        return;
      }
      int switched = 0;
      for (int k = 0; k < std::min(m - 1, 4096); ++k) switched += s[sa[k] - 1] != s[sa[k + 1] - 1];
      if (2 * switched > std::min(m - 1, 4096))
        induce_inplace<true>(s, n, K, cnt.data(), m, sa, bkt.data());
      else
        induce_inplace<false>(s, n, K, cnt.data(), m, sa, bkt.data());
    }
  }
};
}  // namespace diag_split_neo

struct Solver {
  string s;
  vector<int> sa;
  mutable diag_split_neo::State st;

  explicit Solver(const string &s) : s(s), sa(s.size()) {
    st.s = reinterpret_cast<const unsigned char *>(this->s.data()), st.n = int(this->s.size()), st.sa = sa.data();
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
