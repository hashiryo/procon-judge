#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木なので、載せる作用素は
// 1 つに揃える。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<998244353>;

// 反転する操作があるので、和が可換であることを commute で伝える。伝えないと
// 反転したときの積を別に持つことになって、要らない費用が乗る。
struct RaffineSum {
  using T = Mint;
  using E = array<Mint, 2>;  // {b, c} で x を b x + c にする
  static T op(T vl, T vr) { return vl + vr; }
  static void mp(T &v, const E &f, int sz) { v = f[0] * v + f[1] * sz; }
  static void cp(E &pre, const E &suf) {
    pre[0] *= suf[0], pre[1] = suf[0] * pre[1] + suf[1];
  }
  using commute = void;
};

inline vector<Mint> to_mint(const vector<i64> &a) {
  return vector<Mint>(a.begin(), a.end());
}
