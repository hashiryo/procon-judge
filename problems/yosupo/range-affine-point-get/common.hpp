#pragma once
// ライブラリのコンテナを使う提出が共有するもの。比べたいのはコンテナなので、
// 載せる作用素は 1 つに揃える。
//
// ライブラリを使わない実装を足すときは、これを include せず自分で持つ。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<998244353>;

// 1 点しか取らないので値の側のモノイドは要らない。双対だけを渡す。
struct Raffine {
  using T = Mint;
  using E = array<Mint, 2>;  // {b, c} で x を b x + c にする
  static void mp(T &v, const E &f) { v = f[0] * v + f[1]; }
  static void cp(E &pre, const E &suf) {
    pre = {suf[0] * pre[0], suf[0] * pre[1] + suf[1]};
  }
};

inline vector<Mint> to_mint(const vector<i64> &a) {
  return vector<Mint>(a.begin(), a.end());
}
