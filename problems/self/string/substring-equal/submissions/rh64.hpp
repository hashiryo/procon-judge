#pragma once
// 法 2^64 (u64 の桁あふれのまま) のローリングハッシュ。掛け算の剰余が要らないので速いが、Thue–Morse 列の長さ 2^k (k >= 11) の
// 塊とその a と b を入れ替えた塊は、どの基数でもハッシュが一致する (差が (B - 1)(B^2 - 1)(B^4 - 1)... の形で 2^64 で割り切れる)。
// その組を比べるケース (seed 6) で誤ることを示すために置いた。
#include "pj.hpp"
#include <random>

struct Solver {
  vector<u64> h, pw;
  explicit Solver(const string &s) {
    const int n = (int)s.size();
    const u64 B = (random_device{}() * 0x9E3779B97F4A7C15ull) | 1;
    h.resize(n + 1), pw.resize(n + 1);
    h[0] = 0, pw[0] = 1;
    for (int i = 0; i < n; ++i) h[i + 1] = h[i] * B + (unsigned char)s[i], pw[i + 1] = pw[i] * B;
  }
  bool equal(int a, int b, int len) { return h[a + len] - h[b + len] == (h[a] - h[b]) * pw[len]; }
};
