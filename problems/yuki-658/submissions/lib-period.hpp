#pragma once
#include "pj.hpp"
#include "mylib/misc/Period.hpp"

// 直前 4 項を状態にした写像の軌道を辿って周期を見つけ、n だけ進んだ先を
// 尻尾と周期の長さから飛ぶ。状態は 17^4 通りまでなので構築はその範囲で、
// 質問ごとは O(1) に近い。
struct Solver {
  vector<i64> ns, ans;

  explicit Solver(const vector<i64> &ns) : ns(ns) {}

  void run() {
    using Dat = array<int, 4>;
    auto f = [](const Dat &x) -> Dat {
      return {x[1], x[2], x[3], (x[0] + x[1] + x[2] + x[3]) % 17};
    };
    const Dat init{0, 0, 0, 1};
    Period<Dat> p(f, {init});
    ans.clear();
    for (i64 n : ns) ans.push_back(p.jump(init, n - 1)[0]);
  }

  const vector<i64> &answer() const { return ans; }
};
