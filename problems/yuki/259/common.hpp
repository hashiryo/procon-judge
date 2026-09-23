#pragma once
// ライブラリを使う提出が共有するもの。魚の動きを 2 本の列の回転と反転で表す
// 手順はどの実装でも同じなので、ここに置く。比べたいのは反転と分割と結合が
// できる平衡二分木の実装。
#include <utility>
#include "pj.hpp"

struct RangeSum {
  using T = i64;
  static T op(T a, T b) { return a + b; }
  using commute = void;
};

// Tree は RangeSum を載せて反転できる列で、次を持つ。
//   Tree(int n, i64 v);                  // 長さ n、全部 v
//   pair<Tree, Tree> split(int k);       // 先頭 k 個とそれ以外
//   Tree operator+(Tree) const;          // 結合
//   void reverse();
//   void mul(int i, i64 v);              // i 番目に v を足す
//   i64 prod(int l, int r);              // [l, r) の和
template <class Tree> struct FishSolver {
  int n;
  Tree left, right;  // 左向きの魚の列と右向きの魚の列。位置 x の魚は添字 x
  i64 now = 0;

  explicit FishSolver(int n) : n(n), left(n, 0), right(n, 0) {}

  // 時刻 t まで進める。dt 進むと左向きの列は左へ dt ずれ、はみ出た先頭の dt 個は
  // 反転して右向きの列の先頭になる。右向きも対称。2N 進むと元に戻り、N 進むと
  // 左右がそっくり入れ替わるので、dt は N 以内に畳む。
  void advance(i64 t) {
    int dt = (int)((t - now) % (2 * n));
    if (dt > n) std::swap(left, right), left.reverse(), right.reverse(), dt -= n;
    auto [ll, lr] = left.split(dt);
    auto [rl, rr] = right.split(n - dt);
    ll.reverse(), rr.reverse();
    left = lr + rr;
    right = ll + rl;
    now = t;
  }

  void add(i64 t, int y, i64 z, bool to_right) {
    advance(t);
    (to_right ? right : left).mul(y, z);
  }

  i64 count(i64 t, int y, int z) {
    advance(t);
    return left.prod(y, z) + right.prod(y, z);
  }
};
