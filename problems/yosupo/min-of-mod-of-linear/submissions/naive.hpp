#pragma once
// 愚直。x を 0 から n-1 まで動かして最小を取る。小さいケースの突き合わせ用。
#include <cstdint>

struct Solver {
  unsigned long long n, m, a, b, res = 0;
  Solver(unsigned long long n_, unsigned long long m_, unsigned long long a_,
         unsigned long long b_)
      : n(n_), m(m_), a(a_), b(b_) {}
  void run() {
    unsigned long long v = b % m, best = v;
    for (unsigned long long x = 1; x < n; ++x) {
      v = (v + a) % m;
      if (v < best) best = v;
    }
    res = best;
  }
  unsigned long long answer() const { return res; }
};
