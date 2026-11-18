#pragma once
// 今の Library の matroid_intersection (交換の判定を 2 引数で呼び、最短の増加路を層ごとにまとめて探す Cunningham の方法)
// と PartitionMatroid を使う。Library に GF(2) の線形マトロイドは無いので、同じ形 (build、oracle(e)、oracle(e, f)) で
// ここに書く。
#include "mylib/optimization/matroid_intersection.hpp"

struct LibXorMatroid {
  const vector<u64> *v;
  vector<int> pos;
  u64 row[64] = {}, mask[64] = {};
  explicit LibXorMatroid(const vector<u64> &vec) : v(&vec), pos(vec.size(), -1) {}
  void build(const vector<int> &I) {
    for (int b = 0; b < 64; ++b) row[b] = 0;
    for (int k = 0; k < (int)I.size(); ++k) {
      pos[I[k]] = k;
      u64 x = (*v)[I[k]], m = 1ULL << k;
      while (x) {
        const int b = 63 - __builtin_clzll(x);
        if (!row[b]) {
          row[b] = x, mask[b] = m;
          break;
        }
        x ^= row[b], m ^= mask[b];
      }
    }
  }
  u64 reduce(int f) const {
    u64 x = (*v)[f], m = 0;
    while (x) {
      const int b = 63 - __builtin_clzll(x);
      if (!row[b]) return ~0ULL;
      x ^= row[b], m ^= mask[b];
    }
    return m;
  }
  bool oracle(int e) const { return reduce(e) == ~0ULL; }
  bool oracle(int e, int f) const {
    const u64 m = reduce(f);
    return m == ~0ULL || (m >> pos[e] & 1);
  }
};

struct Solver {
  const vector<u64> &vec;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : vec(vec), color(color), cap(cap) {}

  void run() {
    const int m = vec.size();
    LibXorMatroid x(vec);
    vector<vector<int>> parts(cap.size());
    for (int i = 0; i < m; ++i) parts[color[i]].push_back(i);
    PartitionMatroid p(m, parts, cap);
    ans = matroid_intersection(m, x, p);
  }

  vector<int> answer() const { return ans; }
};
