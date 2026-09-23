#pragma once
// ライブラリを使う提出が共有するもの。比べたいのは永続な構造の持ち方なので、
// 解法はここに 1 つ置いて、提出は構造を選ぶだけにする。
#include "pj.hpp"

// 値は int に収まる。ti は動的セグメント木が触っていない添字に返す値で、
// 重み平衡木は半群だけを見るので余っても害はない。
struct RangeMin {
  using T = int;
  static T ti() { return 1 << 30; }
  static T op(T l, T r) { return l < r ? l : r; }
};

inline vector<int> to_int(const vector<i64> &a) {
  return vector<int>(a.begin(), a.end());
}

template <class Tree> struct SeqSolver {
  Tree t[2];

  SeqSolver(const vector<i64> &a, const vector<i64> &b)
      : t{Tree(to_int(a)), Tree(to_int(b))} {}

  void set(int k, int i, i64 v) { t[k].set(i, (int)v); }

  i64 min_of(int k, int l, int r) { return t[k].prod(l, r); }

  // 永続なので、複製ではなく根を差し替えるだけで済む。
  void assign(int dst, int src) { t[dst] = t[src]; }
};
