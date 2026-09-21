#pragma once
#include "pj.hpp"
#include "mylib/misc/Period.hpp"

// 直前 3 項の 1 の位を状態にした写像の軌道を辿って周期を見つけ、K だけ進んだ
// 先を尻尾と周期の長さから飛ぶ。状態は 10^3 通りまで。
struct Solver {
  i64 p, q, r, k, ans = 0;

  Solver(i64 p, i64 q, i64 r, i64 k) : p(p), q(q), r(r), k(k) {}

  void run() {
    using Dat = array<int, 3>;
    auto f = [](const Dat &x) -> Dat { return {x[1], x[2], (x[0] + x[1] + x[2]) % 10}; };
    const Dat init{(int)(p % 10), (int)(q % 10), (int)(r % 10)};
    Period<Dat> period(f, {init});
    ans = period.jump(init, k - 1)[0];
  }

  i64 answer() const { return ans; }
};
