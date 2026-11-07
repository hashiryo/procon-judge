#pragma once
#include "_mont.hpp"

// 2 本のスタックで持つ両端のスライド窓の集約。列の順は「前のスタックの頂上から底、後ろのスタックの
// 底から頂上」で、前のスタックは各段に「その段から底までの合成」、後ろのスタックは「底からその段
// までの合成」を持つ。片方が空のときに取り出すと、もう片方の要素の半分を移して両方の合成を作り
// 直す (ならして O(1))。値は Montgomery の u32 の組。
struct Solver {
  struct F {
    u32 m, a;
  };
  static F op(F l, F r) { return {mont::mul(r.m, l.m), mont::mulsum(r.m, l.a, r.a, mont::R1)}; }
  static u32 apply(F f, u32 x) { return mont::mulsum(f.m, x, f.a, mont::R1); }
  vector<F> fe, fa, be, ba, tmp;

  explicit Solver(int q) {
    for (auto *v : {&fe, &fa, &be, &ba, &tmp}) v->reserve(q);
  }
  void add_front(F f) {
    fe.push_back(f);
    fa.push_back(fa.empty() ? f : op(f, fa.back()));
  }
  void add_back(F f) {
    be.push_back(f);
    ba.push_back(ba.empty() ? f : op(ba.back(), f));
  }
  void push_front(i64 a, i64 b) { add_front({mont::enc(a), mont::enc(b)}); }
  void push_back(i64 a, i64 b) { add_back({mont::enc(a), mont::enc(b)}); }
  // 後ろのスタックの前半 (列で前の方) を前のスタックへ移す。
  void refill_front() {
    const int k = (int)be.size(), h = (k + 1) / 2;
    tmp.assign(be.begin() + h, be.end());
    fe.clear(), fa.clear();
    for (int i = h - 1; i >= 0; --i) add_front(be[i]);
    be.clear(), ba.clear();
    for (const F &f : tmp) add_back(f);
  }
  // 前のスタックの底の側 (列で後ろの方) の半分を後ろのスタックへ移す。
  void refill_back() {
    const int k = (int)fe.size(), h = (k + 1) / 2;
    tmp.assign(fe.begin() + h, fe.end());
    be.clear(), ba.clear();
    for (int i = h - 1; i >= 0; --i) add_back(fe[i]);
    fe.clear(), fa.clear();
    for (const F &f : tmp) add_front(f);
  }
  void pop_front() {
    if (fe.empty()) refill_front();
    fe.pop_back(), fa.pop_back();
  }
  void pop_back() {
    if (be.empty()) refill_back();
    be.pop_back(), ba.pop_back();
  }
  i64 query(i64 x) {
    u32 v = mont::enc(x);
    if (!fa.empty()) v = apply(fa.back(), v);
    if (!ba.empty()) v = apply(ba.back(), v);
    return mont::dec(v);
  }
};
