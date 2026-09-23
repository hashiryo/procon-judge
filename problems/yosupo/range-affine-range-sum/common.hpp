#pragma once
// ライブラリのコンテナを使う提出が共有するもの。
//
// 比べたいのはコンテナなので、載せる作用素モノイドは 1 つに揃える。元の
// Library のテストは E を array にしたものと pair にしたものが混ざっていて、
// そのままだと持ち方の違いまで測定に乗る。
//
// ライブラリを使わない実装を足すときは、これを include せず自分で持つ。
// ここは mylib の ModInt に依存している。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<998244353>;

struct RaffineSum {
  using T = Mint;
  using E = array<Mint, 2>;  // {b, c} で x を b x + c にする
  static T ti() { return Mint(); }
  static T op(const T &l, const T &r) { return l + r; }
  // 和に作用させるので、区間の長さぶん c を足す。
  static void mp(T &v, const E &f, int sz) { v = f[0] * v + f[1] * sz; }
  static void cp(E &pre, const E &suf) {
    pre[0] *= suf[0], pre[1] = suf[0] * pre[1] + suf[1];
  }
};

inline vector<Mint> to_mint(const vector<i64> &a) {
  return vector<Mint>(a.begin(), a.end());
}
