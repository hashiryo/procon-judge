#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木の持ち方なので、載せる
// モノイドは 1 つに揃える。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<998244353>;

// {a, b} が a x + b を表す。l を先に適用してから r を適用する順で合成する。
// 可換でないので、Link-Cut 木の側は反転したときの積も持つことになる。
struct Composite {
  using T = array<Mint, 2>;
  static T ti() { return {Mint(1), Mint()}; }
  static T op(const T &l, const T &r) {
    return {l[0] * r[0], l[1] * r[0] + r[1]};
  }
};
