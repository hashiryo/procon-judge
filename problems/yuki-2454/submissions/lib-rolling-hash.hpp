#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "mylib/string/RollingHash.hpp"

// ローリングハッシュ (998244353 の 2 本)。部分文字列どうしの比較は、ハッシュの
// 一致を二分探索して最長共通接頭辞を出し、次の 1 文字で決める。切る位置ごとに
// O(log N)。
struct Solver {
  vector<string> cases;
  vector<i64> ans;

  explicit Solver(const vector<string> &cases) : cases(cases) {}

  void run() {
    using Mint = ModInt<998244353>;
    using K = Pointwise<Mint, Mint>;
    using RH = RollingHash<K>;
    RH::init({rng(), rng()});
    ans.clear();
    for (auto &s : cases) {
      RH rh(s);
      const int n = (int)s.size();
      i64 cnt = 0;
      for (int i = 1; i < n; ++i) cnt += rh.sub(0, i) < rh.sub(i);
      ans.push_back(cnt);
    }
  }

  const vector<i64> &answer() const { return ans; }
};
