#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは木の持ち方なので、載せる
// 作用付きモノイドは 1 つに揃える。
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"

using Mint = ModInt<1000000007>;

// {宿泊費の和, インフレ係数の和}。規模 z のパレードは s += c * z なので、係数の
// 和を持っていれば区間にまとめて作用できる。作用の合成は z の和。
struct Inflation {
  struct T {
    Mint s, c;
  };
  using E = Mint;
  static T ti() { return {Mint(), Mint()}; }
  static T op(const T &l, const T &r) { return {l.s + r.s, l.c + r.c}; }
  static void mp(T &v, const E &z) { v.s += v.c * z; }
  static void cp(E &p, const E &z) { p += z; }
  using commute = void;
};
