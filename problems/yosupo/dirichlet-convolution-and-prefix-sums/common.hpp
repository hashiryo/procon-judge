#pragma once
// 商の列 Q_N = { floor(N/i) } を昇順に持つ。添字は x <= K なら x-1、そうでなければ len - N/x。
#include "pj.hpp"

constexpr u32 DC_MOD = 998244353;

struct Quotients {
  u64 n, k;
  vector<u64> asc;  // 昇順の Q_N
  size_t len() const { return asc.size(); }
  size_t index(u64 x) const { return x <= k ? (size_t)x - 1 : asc.size() - n / x; }
  u64 value(size_t i) const { return asc[i]; }
};

inline Quotients make_quotients(u64 n) {
  Quotients q;
  q.n = n;
  q.k = (u64)std::sqrt((double)n);
  while (q.k && q.k > n / q.k) --q.k;
  while ((q.k + 1) <= n / (q.k + 1)) ++q.k;
  q.asc.reserve(q.k * 2);
  for (u64 v = 1; v <= q.k; ++v) q.asc.push_back(v);
  for (u64 i = q.k; i >= 1; --i) {
    const u64 v = n / i;
    if (v > q.k) q.asc.push_back(v);
  }
  return q;
}
