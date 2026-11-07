#pragma once
// 法 2^61 - 1 のローリングハッシュで、LCP の長さを 1、2、4、... と倍にして範囲を絞ってから二分探索する形。長さ L まで等しいかは
// h[i + L] - h[j + L] = (h[i] - h[j]) B^L で見る (掛け算 1 回)。LCP が L なら 2 log L 回ほど比べる。先頭の 1 文字が違えばハッシュを
// 見ずに 0 を返す。
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
  const string &s;
  int n;
  vector<u64> h, pw;
  explicit Solver(const string &s) : s(s), n((int)s.size()) {
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
    const int lim = n - max(i, j);
    if (i == j) return lim;
    if (s[i] != s[j]) return 0;
    const u64 d = sub(h[i], h[j]);
    auto eq = [&](int m) { return sub(h[i + m], h[j + m]) == mul(d, pw[m]); };
    int ok = 1, ng = lim + 1;
    for (int m = 2; m <= lim; m *= 2) {
      if (!eq(m)) {
        ng = m;
        break;
      }
      ok = m;
    }
    while (ng - ok > 1) {
      const int m = (ok + ng) / 2;
      (eq(m) ? ok : ng) = m;
    }
    return ok;
  }
};
