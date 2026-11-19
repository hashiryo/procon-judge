#pragma once
// 直線の集合で解く提出が共有する DP。比べたいのは直線の集合の最小を取る道具。
#include "pj.hpp"

template <class Lines, class Gen> Lines make_lines(Gen gen) {
  if constexpr (std::is_constructible_v<Lines, const vector<i64> &>) return Lines(gen());
  else return Lines();
}

// Lines は次を実装する (最小化)。
//   void insert(i64 a, i64 b);  // 直線 y = a x + b を足す。傾き a は足す順に減る
//   i64 query(i64 x);           // x での最小値。x は問うたびに増える
// 問いの x を先に使う Lines は、x の列を受けるコンストラクタ Lines(const vector<i64> &xs) を持てる。持って
// いれば、run() の中で問いの x の列 (h_2, ..., h_N) を作って渡す。
template <class Lines> struct FrogSolver {
  i64 c;
  vector<i64> h;
  i64 ans = 0;

  FrogSolver(i64 c, const vector<i64> &h) : c(c), h(h) {}

  void run() {
    const int n = (int)h.size();
    Lines lines = make_lines<Lines>([&] { return vector<i64>(h.begin() + 1, h.end()); });
    i64 dp = 0;
    lines.insert(-2 * h[0], h[0] * h[0]);
    for (int i = 1; i < n; ++i) {
      dp = lines.query(h[i]) + h[i] * h[i] + c;
      lines.insert(-2 * h[i], dp + h[i] * h[i]);
    }
    ans = dp;
  }

  i64 answer() const { return ans; }
};
