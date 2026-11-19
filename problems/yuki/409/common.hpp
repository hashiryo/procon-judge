#pragma once
// ライブラリを使う提出が共有するもの。費用関数はどちらも同じなので、ここに
// 置いて解き方だけを変える。
#include <algorithm>
#include "pj.hpp"

// 最後に i 日目から N 日目まで運動を続けたときの増減。
inline i64 tail(i64 a, i64 b, i64 n, i64 i) {
  return b * (n - i) * (n - i + 1) / 2 - a * (n - i);
}

template <class Lines, class Gen> Lines make_lines(Gen gen) {
  if constexpr (std::is_constructible_v<Lines, const vector<i64> &>) return Lines(gen());
  else return Lines();
}

// 直線の集合で解く提出が共有する DP。Lines は次を実装する (最小化)。
//   void insert(i64 a, i64 b);  // 直線 y = a x + b を足す。傾き a は足す順に増えない
//   i64 query(i64 x);           // x での最小値。x は問うたびに 1 ずつ増える
// 問いの x を先に使う Lines は、x の列を受けるコンストラクタ Lines(const vector<i64> &xs) を持てる。
//
// 最後に運動を始めた日を j として、i 日目までの最小を dp_i とする。費用 b k (k - 1) / 2 - a (k - 1) + d_i
// (k = i - j) を展開すると、j ごとの直線 y = -b j x + (dp_j + b j (j + 1) / 2 + a j) を x = i で評価した最小に、
// i だけの項 d_i + b i (i - 1) / 2 - a i + a を足したものになる。b ≥ 0 なので傾きは j について増えない。
template <class Lines> struct LineSolver {
  i64 a, b, w;
  vector<i64> d;
  i64 ans = 0;

  LineSolver(i64 a, i64 b, i64 w, const vector<i64> &d) : a(a), b(b), w(w), d(d) {}

  void run() {
    const i64 n = (i64)d.size();
    Lines lines = make_lines<Lines>([&] {
      vector<i64> xs(n);
      for (i64 i = 1; i <= n; ++i) xs[i - 1] = i;
      return xs;
    });
    ans = tail(a, b, n, 0);
    lines.insert(0, 0);
    for (i64 i = 1; i <= n; ++i) {
      const i64 dp = lines.query(i) + d[i - 1] + b * i * (i - 1) / 2 - a * i + a;
      ans = std::min(ans, dp + tail(a, b, n, i));
      lines.insert(-b * i, dp + b * i * (i + 1) / 2 + a * i);
    }
    ans += w;
  }

  i64 answer() const { return ans; }
};
