#pragma once
// 閉路の列挙 (circuit) で交換グラフを毎回作り、増加路を 1 本ずつ探す方法 (intersect_circuit)。
// M1 を GF(2) の線形マトロイド、M2 を分割マトロイドにする。マトロイドの型と算法は _shared/matroid/ にある。
#include "_shared/matroid/matroids.hpp"
#include "_shared/matroid/mi_algos.hpp"

struct Solver {
  const vector<u64> &vec;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : vec(vec), color(color), cap(cap) {}

  void run() {
    mtr::Xor64 x(vec);
    mtr::Partition p(color, cap);
    ans = mi_algo::intersect_circuit((int)vec.size(), x, p);
  }

  vector<int> answer() const { return ans; }
};
