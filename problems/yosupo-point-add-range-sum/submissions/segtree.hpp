#pragma once
#include "common.hpp"

// 非再帰のセグメント木。葉を [n, 2n) に置いて、親を i -> 2i, 2i+1 で持つ。
// Fenwick tree と同じ O(log N) だが、1 クエリで触るノードが倍あり、
// 区間取得で左右から詰める分だけ分岐が増える。
struct Solver {
  int n;
  vector<i64> t;

  explicit Solver(const vector<i64> &init) : n((int)init.size()), t(2 * n) {
    for (int i = 0; i < n; ++i) t[n + i] = init[i];
    for (int i = n - 1; i > 0; --i) t[i] = t[2 * i] + t[2 * i + 1];
  }

  void add(int p, i64 x) {
    for (t[p += n] += x; p > 1; p >>= 1) t[p >> 1] = t[p] + t[p ^ 1];
  }

  i64 sum(int l, int r) {
    i64 s = 0;
    for (l += n, r += n; l < r; l >>= 1, r >>= 1) {
      if (l & 1) s += t[l++];
      if (r & 1) s += t[--r];
    }
    return s;
  }
};
