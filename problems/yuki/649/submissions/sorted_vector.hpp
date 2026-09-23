#pragma once
#include <algorithm>

#include "pj.hpp"

// 整列した配列をそのまま持つ。insert は挿入位置を二分探索してから後ろをずらす
// ので O(N)。K 番目は添字を引くだけで O(1)。比較の下限を置くための実装で、
// 大きいケースでは要素の移動が効いてくる。
struct Solver {
  i64 k;
  vector<i64> a;

  explicit Solver(i64 k_) : k(k_) {}

  void insert(i64 x) { a.insert(lower_bound(a.begin(), a.end(), x), x); }

  i64 pop_kth() {
    if ((i64)a.size() < k) return -1;
    i64 ans = a[k - 1];
    a.erase(a.begin() + (k - 1));
    return ans;
  }
};
