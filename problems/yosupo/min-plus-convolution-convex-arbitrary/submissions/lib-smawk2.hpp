#pragma once
#include <algorithm>
#include <numeric>
#include "pj.hpp"

// totally monotone な行列の各行の最小を SMAWK で求める O(H+W) の、lib-smawk を詰めた版。
// - 列を減らす段は、同じ (行, 山の頂上) の値を覚えて 2 回目の評価を省く。
// - 偶数番目の行の走査は、山の範囲を有効な列 [lo, hi] に切り落としてから、1 候補 1 評価の
//   分岐の少ない最小更新にする (lib-smawk は無効な列も INF で評価していた)。
struct Solver {
  vector<i64> a, b, c;
  int n = 0, m = 0;
  static constexpr i64 INF = numeric_limits<i64>::max() / 4;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  [[gnu::always_inline]] i64 val(int k, int j) const {
    const unsigned i = (unsigned)(k - j);
    return i < (unsigned)n ? a[i] + b[j] : INF;
  }

  void rec(const vector<int> &rows, const vector<int> &cols, vector<int> &ans) const {
    if (rows.empty()) return;
    // 列を減らす。
    vector<int> stk;
    stk.reserve(cols.size());
    int crow = -1, ctop = -1;
    i64 cval = 0;
    for (int col : cols) {
      while (!stk.empty()) {
        const int row = rows[(int)stk.size() - 1], top = stk.back();
        i64 topv;
        if (row == crow && top == ctop) topv = cval;
        else topv = cval = val(row, top), crow = row, ctop = top;
        if (val(row, col) <= topv) stk.pop_back();
        else break;
      }
      if ((int)stk.size() < (int)rows.size()) stk.push_back(col);
    }
    // 奇数番目の行を解く。
    vector<int> odd;
    odd.reserve(rows.size() / 2);
    for (size_t i = 1; i < rows.size(); i += 2) odd.push_back(rows[i]);
    rec(odd, stk, ans);
    // 偶数番目の行は、両隣の答えの間を、有効な列だけ見る。
    size_t pos = 0;
    for (size_t i = 0; i < rows.size(); i += 2) {
      const int hm = rows[i];
      size_t lp = 0;
      if (i > 0) {
        while (stk[pos] != ans[rows[i - 1]]) ++pos;
        lp = pos;
      } else {
        pos = 0;
      }
      size_t hp = stk.size() - 1;
      if (i + 1 < rows.size()) {
        size_t q = pos;
        while (stk[q] != ans[rows[i + 1]]) ++q;
        hp = q;
      }
      const int lo = std::max(0, hm - (n - 1)), hi = std::min(m - 1, hm);
      while (lp < hp && stk[lp] < lo) ++lp;
      while (hp > lp && stk[hp] > hi) --hp;
      int best = stk[lp];
      i64 bestv = a[hm - best] + b[best];
      for (size_t t = lp + 1; t <= hp; ++t) {
        const int j = stk[t];
        const i64 v = a[hm - j] + b[j];
        if (v < bestv) bestv = v, best = j;
      }
      ans[hm] = best;
    }
  }

  void run() {
    n = (int)a.size(), m = (int)b.size();
    const int h = n + m - 1;
    vector<int> rows(h), cols(m), ans(h);
    iota(rows.begin(), rows.end(), 0), iota(cols.begin(), cols.end(), 0);
    rec(rows, cols, ans);
    c.resize(h);
    for (int k = 0; k < h; ++k) c[k] = a[k - ans[k]] + b[ans[k]];
  }

  const vector<i64> &answer() const { return c; }
};
