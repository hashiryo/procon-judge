#pragma once
// 法 2^61 - 1 のローリングハッシュで、1 つのクエリの掛け算を 1 回にしたもの。s[a, a + len) = s[b, b + len) は
// h[a + len] - h[a] B^len = h[b + len] - h[b] B^len と同じなので、h[a + len] - h[b + len] = (h[a] - h[b]) B^len を比べる。
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
  vector<u64> h, pw;
  explicit Solver(const string &s) {
    const int n = (int)s.size();
    const u64 B = (random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256;
    h.resize(n + 1), pw.resize(n + 1);
    h[0] = 0, pw[0] = 1;
    for (int i = 0; i < n; ++i) {
      const u64 x = mul(h[i], B) + (unsigned char)s[i];
      h[i + 1] = x >= M ? x - M : x;
      pw[i + 1] = mul(pw[i], B);
    }
  }
  bool equal(int a, int b, int len) { return sub(h[a + len], h[b + len]) == mul(sub(h[a], h[b]), pw[len]); }
};
