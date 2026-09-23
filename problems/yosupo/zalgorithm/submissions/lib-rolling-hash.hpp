#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "mylib/string/RollingHash.hpp"

// ローリングハッシュで各接尾辞との最長共通接頭辞を二分探索する O(N log N)。
// Z 配列を知らなくても解けることの確認で、専用の実装に対する上界になる。
// 基数は乱択なので、ごく低い確率で誤る。
struct Solver {
  using Mint = ModInt<998244353>;
  using K = Pointwise<Mint, Mint>;
  using RH = RollingHash<K>;

  string s;
  vector<int> z;

  explicit Solver(const string &s) : s(s) { RH::init({rng(), rng()}); }

  void run() {
    RH rh(s);
    int n = (int)s.size();
    z.resize(n);
    for (int i = 0; i < n; ++i) z[i] = lcp(rh, rh.sub(i));
  }

  const vector<int> &answer() const { return z; }
};
