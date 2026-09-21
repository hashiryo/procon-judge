#pragma once
// 提出が共有するもの。制約の出し方はどの解き方でも同じなので、ここに置いて
// 解き方だけを変える。
#include "pj.hpp"

// 2 つの区間が重なるか。
inline bool overlap(i64 l1, i64 r1, i64 l2, i64 r2) {
  return !(r1 < l2 || r2 < l1);
}

// ブロック i と j の間の制約。裏返しは鏡映なので、両方そのままで重なるなら両方
// 裏返しても重なり、片方だけ裏返して重なるなら逆の片方だけ裏返しても重なる。
//   differ: 両方そのままで重なる     -> x_i != x_j でなければならない
//   same:   片方だけ裏返すと重なる   -> x_i = x_j でなければならない
// 両方立てば矛盾で、どちらも立たなければ制約は無い。
struct Relation {
  bool same, differ;
};

inline Relation constraint(int m, i64 li, i64 ri, i64 lj, i64 rj) {
  return {overlap(li, ri, m - 1 - rj, m - 1 - lj), overlap(li, ri, lj, rj)};
}
