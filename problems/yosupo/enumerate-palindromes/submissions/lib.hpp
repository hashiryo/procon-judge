#pragma once
// 今の Library (mylib) のローリングハッシュ (mylib/string/RollingHash.hpp) で、中心ごとに、文字列とその反転の LCP を二分探索で
// 求める提出 (O(n log n))。raw のときの lib.cpp と同じ計算で、法 998244353 の 2 つの基数の組をハッシュにする。
// Z 配列と回文の実装を比べた記録は algo-notes の notes/z-array-palindromes.md にある。
#include <algorithm>
#include <string>
#include <vector>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/string/RollingHash.hpp"
#include "mylib/misc/rng.hpp"

struct Solver {
  string s;
  vector<int> len;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using Mint = ModInt<998244353>;
    using K = Pointwise<Mint, Mint>;
    using RH = RollingHash<K>;
    RH::init({rng(), rng()});
    const int N = int(s.size());
    RH rh(s);
    string r(s.rbegin(), s.rend());
    RH rhrv(r);
    len.resize(2 * N - 1);
    for (int i = 0; i < N + N - 1; ++i) {
      const int n = lcp(rh.sub((i + 1) / 2), rhrv.sub((N + N - i - 1) / 2));
      len[i] = n * 2 - !(i & 1);
    }
  }

  const vector<int> &answer() const { return len; }
};
