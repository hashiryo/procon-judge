#pragma once
#include "common.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "mylib/string/RollingHash.hpp"

// ローリングハッシュ (998244353 の 2 本)。接尾辞ごとに、ハッシュの一致を二分探索
// して最長共通接頭辞を出す。M 本で O(M log N)。
struct Match {
  static vector<int> prefix_matches(const vector<int> &a, const vector<int> &b) {
    using Mint = ModInt<998244353>;
    using K = Pointwise<Mint, Mint>;
    using RH = RollingHash<K>;
    RH::init({rng(), rng()});
    RH rha(a), rhb(b);
    vector<int> z(b.size());
    for (int i = 0; i < (int)b.size(); ++i) z[i] = lcp(rha, rhb.sub(i));
    return z;
  }
};

using Solver = SequenceSolver<Match>;
