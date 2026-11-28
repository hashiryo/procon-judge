#pragma once
#include <numeric>
#include "pj.hpp"

// totally monotone な行列の各行の最小を SMAWK で求める O(H+W)。行列は作らず、値を返す関数だけを渡す。
// 凸な a と任意の b の min-plus convolution の行列は totally monotone になる。
// 行の最小の列が行について単調なので、列を減らしながら奇数番目の行で再帰し、偶数番目の行は両隣の間だけを見る。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size(), h = n + m - 1;
    static constexpr i64 INF = numeric_limits<i64>::max() / 4;
    auto f = [&](int k, int j) -> i64 {
      const int i = k - j;
      if (i < 0 || i >= n) return INF;
      return a[i] + b[j];
    };
    vector<int> rows(h), cols(m), ans(h);
    iota(rows.begin(), rows.end(), 0), iota(cols.begin(), cols.end(), 0);
    rec(f, rows, cols, ans);
    c.resize(h);
    for (int k = 0; k < h; ++k) c[k] = f(k, ans[k]);
  }

  template <class F>
  static void rec(const F &f, const vector<int> &rows, const vector<int> &cols, vector<int> &ans) {
    if (rows.empty()) return;
    // 列を減らす。各行の最小になりえない列を落とす。
    vector<int> stk;
    stk.reserve(cols.size());
    for (int col : cols) {
      while (!stk.empty()) {
        const int row = rows[(int)stk.size() - 1];
        if (f(row, col) <= f(row, stk.back())) stk.pop_back();
        else break;
      }
      if ((int)stk.size() < (int)rows.size()) stk.push_back(col);
    }
    // 奇数番目の行を解く。
    vector<int> odd;
    odd.reserve(rows.size() / 2);
    for (size_t i = 1; i < rows.size(); i += 2) odd.push_back(rows[i]);
    rec(f, odd, stk, ans);
    // 偶数番目の行は、両隣の答えの列の間だけを見る。
    size_t pos = 0;
    for (size_t i = 0; i < rows.size(); i += 2) {
      size_t lo = 0;
      if (i > 0) {
        while (stk[pos] != ans[rows[i - 1]]) ++pos;
        lo = pos;
      } else {
        pos = 0;
      }
      size_t hi = stk.size() - 1;
      if (i + 1 < rows.size()) {
        size_t q = pos;
        while (stk[q] != ans[rows[i + 1]]) ++q;
        hi = q;
      }
      int best = stk[lo];
      for (size_t t = lo + 1; t <= hi; ++t)
        if (f(rows[i], stk[t]) <= f(rows[i], best)) best = stk[t];
      ans[rows[i]] = best;
    }
  }

  const vector<i64> &answer() const { return c; }
};
