#pragma once
// 法 2^61 - 1 のローリングハッシュで、LCP の長さを二分探索する。長さ L まで等しいかは
// h[i + L] - h[j + L] = (h[i] - h[j]) B^L で見る (掛け算 1 回)。1 つのクエリで log n 回ほど比べる。
#include "pj.hpp"
#include <random>

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  static u64 mul(u64 a, u64 b) {
    const unsigned __int128 t = (unsigned __int128)a * b;
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
  static u64 sub(u64 a, u64 b) { return a >= b ? a - b : a + M - b; }
  int n;
  vector<u64> h, pw;
  explicit Solver(const string &s) : n((int)s.size()) {
    const u64 B = (random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256;
    h.resize(n + 1), pw.resize(n + 1);
    h[0] = 0, pw[0] = 1;
    for (int i = 0; i < n; ++i) {
      const u64 x = mul(h[i], B) + (unsigned char)s[i];
      h[i + 1] = x >= M ? x - M : x;
      pw[i + 1] = mul(pw[i], B);
    }
  }
  int lcp(int i, int j) {
    const u64 d = sub(h[i], h[j]);
    int ok = 0, ng = n - max(i, j) + 1;
    while (ng - ok > 1) {
      const int m = (ok + ng) / 2;
      (sub(h[i + m], h[j + m]) == mul(d, pw[m]) ? ok : ng) = m;
    }
    return ok;
  }
};
