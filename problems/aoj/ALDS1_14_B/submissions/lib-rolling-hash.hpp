#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/misc/Pointwise.hpp"
#include "mylib/misc/rng.hpp"
#include "mylib/string/RollingHash.hpp"

// 部分文字列のハッシュを O(1) で引けるようにしてから、全位置を突き合わせる。
// 基数は乱択なので、ごく低い確率で誤る。2 つの剰余を組にして確率を下げている。
struct Solver {
  using Mint = ModInt<998244353>;
  using K = Pointwise<Mint, Mint>;
  using RH = RollingHash<K>;

  string t, p;
  vector<int> pos;

  Solver(const string &t, const string &p) : t(t), p(p) { RH::init({rng(), rng()}); }

  void run() {
    RH rt(t), rp(p);
    const int n = (int)p.size(), m = (int)t.size();
    auto want = rp.hash();
    pos.clear();
    for (int i = 0; i + n <= m; ++i)
      if (rt.sub(i, n).hash() == want) pos.push_back(i);
  }

  const vector<int> &answer() const { return pos; }
};
