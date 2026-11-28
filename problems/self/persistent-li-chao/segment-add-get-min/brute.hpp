#pragma once
// 愚直解 (pj testdata crosscheck 用)。版ごとに線分の列を丸ごと写し、問いでは走査する。
// q <= 20 の小さい入力だけを扱う。
#include <algorithm>
#include <climits>
#include <vector>

struct Solver {
  struct Seg {
    long long l, r, a, b;
  };
  std::vector<std::vector<Seg>> ver;
  explicit Solver(int q) {
    ver.reserve(q + 1);
    ver.push_back({});
  }
  void insert(int v, long long l, long long r, long long a, long long b) {
    auto s = ver[v];
    s.push_back({l, r, a, b});
    ver.push_back(std::move(s));
  }
  long long query(int v, long long x) const {
    long long res = LLONG_MAX;
    for (auto &e : ver[v])
      if (e.l <= x && x < e.r) res = std::min(res, e.a * x + e.b);
    return res;
  }
};
