#pragma once
// 愚直解。直前の M - 1 文字 (M はパターンの長さの最大) を状態にした DP。1 文字足すたびに、足したあとの末尾 M 文字以内に終わる
// パターンが無いかを、パターンごとに文字列の比較で確かめる。オートマトンを作らない。小さい入力 (gen.py の seed 1000 以上と seed 0)
// でだけ使う。
#include <map>

struct Solver {
  const vector<string> &ps;
  int sigma;
  Solver(const vector<string> &ps, int sigma) : ps(ps), sigma(sigma) {}
  u32 count(int n) {
    constexpr u64 MOD = 998244353;
    size_t m = 1;
    for (const auto &p : ps) m = max(m, p.size());
    map<string, u64> dp{{"", 1}};
    for (int i = 0; i < n; ++i) {
      map<string, u64> nx;
      for (const auto &[s, w] : dp)
        for (int c = 0; c < sigma; ++c) {
          string t = s + char('a' + c);
          bool ok = true;
          for (const auto &p : ps)
            if (t.size() >= p.size() && t.compare(t.size() - p.size(), p.size(), p) == 0) {
              ok = false;
              break;
            }
          if (!ok) continue;
          if (t.size() > m - 1) t.erase(0, t.size() - (m - 1));
          (nx[t] += w) %= MOD;
        }
      dp.swap(nx);
    }
    u64 s = 0;
    for (const auto &[t, w] : dp) s += w;
    return u32(s % MOD);
  }
};
