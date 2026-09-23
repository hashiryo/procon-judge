#pragma once
// ライブラリを使う提出が共有するもの。DP はどの実装でも同じなので、ここに置く。
// 比べたいのは直線の集合の最小値を取る道具。
#include "pj.hpp"

// Lines は次を実装する (最小化)。
//   void insert(i64 a, i64 b);  // 直線 y = a x + b を足す
//   i64 query(i64 x) const;     // x での最小値
//
// 島を左から順に区間に切り、各区間を 1 人の職人に任せる。区間の左端を決める遷移
// と右端を決める遷移をそれぞれ直線の集合の最小値として持ち、職人 i を見るごとに
// 2 つを交互に更新する (Library の test/yukicoder/1297 の DP をそのまま移した)。
// 費用は 2 倍して整数のまま扱い、最後に半分にする。
template <class Lines> struct StatueSolver {
  int n;
  i64 c;
  vector<array<i64, 2>> ab;
  i64 ans = 0;

  StatueSolver(int n, i64 c, const vector<array<i64, 2>> &ab) : n(n), c(c), ab(ab) {}

  void run() {
    Lines left, right;
    left.insert(0, 0);
    for (i64 i = 1;; ++i) {
      const i64 a = 2 * ab[i - 1][0], b = 2 * ab[i - 1][1], cc = 2 * c * i;
      right.insert(a - cc, left.query(-a - cc) + cc * (i - 1) + b);
      if (i == n) break;
      left.insert(i, right.query(i) + cc * (i + 1));
    }
    ans = (right.query(n) + (i64)(n + 1) * n * c) / 2;
  }

  i64 answer() const { return ans; }
};
