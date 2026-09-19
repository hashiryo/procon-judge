#pragma once
#include <queue>

#include "common.hpp"

// K が全クエリで共通なのを使う。小さい方から K 個を最大ヒープ lo に、残りを
// 最小ヒープ hi に置く。K 番目に小さい値は常に lo の先頭にある。
// insert も pop も O(log Q) で、定数が小さい。
struct Solver {
  i64 k;
  priority_queue<i64> lo;                                   // 小さい方 K 個。先頭が最大
  priority_queue<i64, vector<i64>, greater<i64>> hi;        // 残り。先頭が最小

  explicit Solver(i64 k_) : k(k_) {}

  void insert(i64 x) {
    if ((i64)lo.size() < k) {
      lo.push(x);
    } else if (x < lo.top()) {
      hi.push(lo.top());
      lo.pop();
      lo.push(x);
    } else {
      hi.push(x);
    }
  }

  i64 pop_kth() {
    // lo の大きさは min(|S|, K)。K に満たなければ要素が K 個無い。
    if ((i64)lo.size() < k) return -1;
    i64 ans = lo.top();
    lo.pop();
    if (!hi.empty()) {
      lo.push(hi.top());
      hi.pop();
    }
    return ans;
  }
};
