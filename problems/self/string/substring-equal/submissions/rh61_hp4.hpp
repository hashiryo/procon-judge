#pragma once
// rh61_hp の作る段で、掛け算の鎖を 4 文字ずつにしたもの。h[i + 4] = h[i] B^4 + (c_i B^3 + c_{i+1} B^2 + c_{i+2} B + c_{i+3}) を鎖に
// して、h[i + 1] から h[i + 3] は h[i] から互いに待たずに求める。pw は pw[i] = pw[i - 4] B^4 の 4 本の鎖にする。1 文字ごとの
// 鎖では掛け算と剰余の待ち (10 cycle ほど) が文字の数だけ続く。
#include "pj.hpp"
#include <random>
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  using u128 = unsigned __int128;
  // t < 2^62 M なら M 未満に直す。
  static u64 red(u128 t) {
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
  static u64 mul(u64 a, u64 b) { return red((u128)a * b); }
  static u64 add(u64 a, u64 b) {
    const u64 r = a + b;
    return r >= M ? r - M : r;
  }
  static u64 sub(u64 a, u64 b) { return a >= b ? a - b : a + M - b; }
  string_hp::Vec<u64> h, pw;
  explicit Solver(const string &s) {
    const int n = (int)s.size();
    const u64 B = (random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256;
    const u64 B2 = mul(B, B), B3 = mul(B2, B), B4 = mul(B2, B2);
    h.resize(n + 1), pw.resize(n + 1);
    const unsigned char *c = (const unsigned char *)s.data();
    const u64 p0[4] = {1, B, B2, B3};
    for (int i = 0; i <= n && i < 4; ++i) pw[i] = p0[i];
    for (int i = 4; i <= n; ++i) pw[i] = mul(pw[i - 4], B4);
    h[0] = 0;
    int i = 0;
    for (u64 x = 0; i + 4 <= n; i += 4) {
      const u64 c0 = c[i], c1 = c[i + 1], c2 = c[i + 2], c3 = c[i + 3];
      const u64 q2 = red((u128)c0 * B + c1), q3 = red((u128)c0 * B2 + (u128)c1 * B + c2);
      const u64 q4 = red((u128)c0 * B3 + (u128)c1 * B2 + (u128)c2 * B + c3);
      h[i + 1] = add(mul(x, B), c0), h[i + 2] = add(mul(x, B2), q2), h[i + 3] = add(mul(x, B3), q3);
      x = add(mul(x, B4), q4);
      h[i + 4] = x;
    }
    for (; i < n; ++i) h[i + 1] = add(mul(h[i], B), c[i]);
  }
  bool equal(int a, int b, int len) { return sub(h[a + len], h[b + len]) == mul(sub(h[a], h[b]), pw[len]); }
};
