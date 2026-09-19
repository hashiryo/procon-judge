#pragma once
#include "common.hpp"

// 値の 2 進表現を辿る動的セグメント木。各節点に部分木の個数を持ち、
// 上の桁から降りて K 番目を探す。値は 0 以上 10^18 以下なので 60 桁で足りる。
// insert も pop も O(60)。節点を作るぶんメモリを食う。
struct Solver {
  static constexpr int BITS = 60;

  struct Node {
    int left = -1, right = -1;
    int count = 0;
  };

  i64 k;
  vector<Node> t;

  explicit Solver(i64 k_) : k(k_) { t.push_back(Node{}); }

  void insert(i64 x) {
    int cur = 0;
    t[cur].count++;
    for (int b = BITS - 1; b >= 0; --b) {
      int bit = (int)((x >> b) & 1);
      int next = bit ? t[cur].right : t[cur].left;
      if (next < 0) {
        next = (int)t.size();
        t.push_back(Node{});
        // push_back で参照が無効になるので、足したあとに書き戻す。
        if (bit) t[cur].right = next;
        else t[cur].left = next;
      }
      cur = next;
      t[cur].count++;
    }
    return;
  }

  i64 pop_kth() {
    if (t[0].count < k) return -1;
    i64 need = k, x = 0;
    int cur = 0;
    t[cur].count--;
    for (int b = BITS - 1; b >= 0; --b) {
      int left = t[cur].left;
      i64 in_left = left < 0 ? 0 : t[left].count;
      if (need <= in_left) {
        cur = left;
      } else {
        need -= in_left;
        cur = t[cur].right;
        x |= (i64)1 << b;
      }
      t[cur].count--;
    }
    return x;
  }
};
