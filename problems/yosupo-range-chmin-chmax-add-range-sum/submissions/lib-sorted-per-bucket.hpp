#pragma once
#include "pj.hpp"
#include "mylib/data_structure/SortedPerBucket.hpp"

// 平方分割。バケットごとに整列した列を持って、chmin と chmax は「その値より
// 大きい (小さい) ところだけ潰す」操作として端から適用する。O((N+Q) sqrt N)
// なので Beats より一段遅いが、持つものは値の列だけで済む。
struct Solver {
  SortedPerBucket<i64> buckets;

  explicit Solver(const vector<i64> &a) : buckets(a) {}

  void chmin(int l, int r, i64 b) { buckets.chmin(l, r, b); }

  void chmax(int l, int r, i64 b) { buckets.chmax(l, r, b); }

  void add(int l, int r, i64 b) { buckets.add(l, r, b); }

  i64 sum(int l, int r) { return buckets.sum(l, r); }
};
