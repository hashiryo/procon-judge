#pragma once
// 法 2^61 - 1 のローリングハッシュ。h[i] は s[0, i) のハッシュ、pw[i] は B^i で、基数 B は実行ごとに乱択する。
// s[l, r) のハッシュは h[r] - h[l] B^(r - l) で、1 つのクエリで 2 回掛ける。
#include "pj.hpp"
#include <random>

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  static u64 mul(u64 a, u64 b) {
    const unsigned __int128 t = (unsigned __int128)a * b;
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
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
  u64 get(int l, int r) const {
    const u64 x = h[r] + M - mul(h[l], pw[r - l]);
    return x >= M ? x - M : x;
  }
  bool equal(int a, int b, int len) { return get(a, a + len) == get(b, b + len); }
};
