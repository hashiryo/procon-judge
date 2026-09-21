#pragma once
// 提出が共有するもの。解き方は同じで Union-Find の実装だけが違うので、解法を
// ここにテンプレートで 1 つ置いて、提出は Union-Find の型を選ぶだけにする。
#include "pj.hpp"

constexpr i64 MOD = 998244353;

template <class UF> struct ParitySolver {
  int n;
  vector<array<i64, 3>> claims;
  UF uf;
  i64 ans = 0;

  ParitySolver(int n, const vector<array<i64, 3>> &claims)
      : n(n), claims(claims), uf(n) {}

  void run() {
    for (auto &[a, b, c] : claims)
      if (!uf.unite((int)a, (int)b, (bool)c)) return;  // 矛盾。ans は 0 のまま
    ans = 1;
    for (int i = 0; i < n; ++i)
      if (uf.leader(i) == i) ans = ans * 2 % MOD;
  }

  i64 answer() const { return ans; }
};
