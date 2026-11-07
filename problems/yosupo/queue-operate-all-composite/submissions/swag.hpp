#pragma once
#include "_mont.hpp"

// 2 本のスタックで持つスライド窓の集約 (SWAG)。後ろのスタックは足した 1 次関数をそのまま積み、
// その合成を 1 つ持つ。前のスタックは、後ろから移すときに新しい方から順に積み、各段に「その段から
// 底までの合成」を持つので、頂上が前のスタック全体の合成になる。先頭を消すときに前が空なら後ろを
// 全部移す (ならして O(1))。値は Montgomery の u32 の組。
struct Solver {
  struct F {
    u32 m, a;
  };
  static F op(F l, F r) { return {mont::mul(r.m, l.m), mont::mulsum(r.m, l.a, r.a, mont::R1)}; }
  static u32 apply(F f, u32 x) { return mont::mulsum(f.m, x, f.a, mont::R1); }
  vector<F> back, front;
  F bagg{mont::R1, 0};

  explicit Solver(int q) {
    back.reserve(q);
    front.reserve(q);
  }
  void push(i64 a, i64 b) {
    const F f{mont::enc(a), mont::enc(b)};
    back.push_back(f);
    bagg = op(bagg, f);
  }
  void pop() {
    if (front.empty()) {
      F agg{mont::R1, 0};
      for (int i = (int)back.size() - 1; i >= 0; --i) front.push_back(agg = op(back[i], agg));
      back.clear();
      bagg = {mont::R1, 0};
    }
    front.pop_back();
  }
  i64 query(i64 x) {
    u32 v = mont::enc(x);
    if (!front.empty()) v = apply(front.back(), v);
    return mont::dec(apply(bagg, v));
  }
};
