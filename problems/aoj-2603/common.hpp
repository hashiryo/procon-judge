#pragma once
// ライブラリを使う提出が共有するもの。並べ替えと累積和と費用関数はどの実装
// でも同じなので、ここに置く。比べたいのはこの費用関数の上でどう DP を回すか。
#include <algorithm>
#include "pj.hpp"

struct Cost {
  vector<i64> a, sum;

  explicit Cost(vector<i64> v) : a(std::move(v)) {
    std::sort(a.begin(), a.end());
    sum.assign(a.size() + 1, 0);
    for (size_t i = 0; i < a.size(); ++i) sum[i + 1] = sum[i] + a[i];
  }

  // [j, i) を 1 つの組にしたときの損。組の最大は a[i-1] になる。
  i64 operator()(int i, int j) const {
    return (i64)(i - j) * a[i - 1] - (sum[i] - sum[j]);
  }
};
