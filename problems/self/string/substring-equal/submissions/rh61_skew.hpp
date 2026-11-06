#pragma once
// rh61_hp の h の置き方を、512 個ごとに 1 つずらしたもの (h[i] を i + i / 512 番目に置く)。Thue–Morse のケースのクエリは 2^12 の
// 倍数の位置ばかり読み、8 byte の項では 32 KB おきになって、キャッシュの同じ組に集まる。ずらすと 2^12 個おきの項が組に散らばる。
#include "pj.hpp"
#include <random>
#include "_shared/string/_hugealloc.hpp"

struct Solver {
  static constexpr u64 M = (1ull << 61) - 1;
  static u64 mul(u64 a, u64 b) {
    const unsigned __int128 t = (unsigned __int128)a * b;
    const u64 r = ((u64)t & M) + (u64)(t >> 61);
    return r >= M ? r - M : r;
  }
  static u64 sub(u64 a, u64 b) { return a >= b ? a - b : a + M - b; }
  static size_t at(int i) { return (size_t)i + ((size_t)i >> 9); }
  string_hp::Vec<u64> h, pw;
  explicit Solver(const string &s) {
    const int n = (int)s.size();
    const u64 B = (random_device{}() * 0x9E3779B97F4A7C15ull) % (M - 512) + 256;
    h.resize(at(n) + 1), pw.resize(n + 1);
    h[0] = 0, pw[0] = 1;
    u64 x = 0;
    for (int i = 0; i < n; ++i) {
      const u64 y = mul(x, B) + (unsigned char)s[i];
      x = y >= M ? y - M : y;
      h[at(i + 1)] = x;
      pw[i + 1] = mul(pw[i], B);
    }
  }
  bool equal(int a, int b, int len) { return sub(h[at(a + len)], h[at(b + len)]) == mul(sub(h[at(a)], h[at(b)]), pw[len]); }
};
