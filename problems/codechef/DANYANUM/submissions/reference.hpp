#pragma once
// 参照実装 (ライブラリに依存しない)。値 v の補集合 ~v を数え、上位集合の個数を補集合の部分集合の個数として持つ。bit を下の L = k / 2
// 個と上の残りに分け、T[h][l] = #{要素 e : e の上の部分が h、e の下の部分 ⊆ l} を持つ (下の bit だけゼータ変換)。
//   足す / 消す: e の上の部分の行で、e の下の部分を含む l 全部に ±1 (2^{L - popcount(e の下)} 個)。
//   部分集合の個数 (x): x の上の部分の部分集合 h 全部の T[h][x の下] の和 (2^{popcount(x の上)} 個)。
// 問い f(x) は、上の bit から mask に足せるかを、mask を含む要素の数 (= ~mask の部分集合の個数) が x 以上かで決める。
#include "pj.hpp"
struct Solver {
  int k, L;
  vector<int> a;
  vector<pair<int, int>> ops;
  vector<int> ans, T;
  Solver(int k_, const vector<int> &a_, const vector<pair<int, int>> &ops_) : k(k_), a(a_), ops(ops_) {}
  void upd(int v, int d) {
    const int e = ((1 << k) - 1) ^ v, h = e >> L, lo = e & ((1 << L) - 1), fr = ((1 << L) - 1) ^ lo;
    int *row = &T[size_t(h) << L];
    for (int s = fr;; s = (s - 1) & fr) {
      row[lo | s] += d;
      if (!s) break;
    }
  }
  int cnt(int x) const {  // #{e : e ⊆ x}
    const int h = x >> L, lo = x & ((1 << L) - 1);
    int r = 0;
    for (int s = h;; s = (s - 1) & h) {
      r += T[(size_t(s) << L) | lo];
      if (!s) break;
    }
    return r;
  }
  void run() {
    L = k / 2;
    T.assign(size_t(1) << k, 0);
    for (int v : a) upd(v, 1);
    const int full = (1 << k) - 1;
    for (auto [t, v] : ops) {
      if (t == 1) upd(v, 1);
      else if (t == 2) upd(v, -1);
      else {
        int m = 0;
        for (int b = k - 1; b >= 0; --b)
          if (cnt(full ^ (m | (1 << b))) >= v) m |= 1 << b;
        ans.push_back(m);
      }
    }
  }
  vector<int> answer() const { return ans; }
};
