#pragma once
// 参照実装。今の Library の mylib/optimization/matroid_intersection.hpp (Library 459f66b81) の matroid_intersection と
// PartitionMatroid を写し、GF(2) の線形マトロイドを同じ形 (build、oracle(e)、oracle(e, f)) で足したもの。Library を
// 直しても期待出力が変わらないように、include せずに持つ。
#include <algorithm>
#include <array>
#include <vector>

namespace ref_mi {
template <typename Matroid1, typename Matroid2> std::vector<int> matroid_intersection(int n, Matroid1 M1, Matroid2 M2) {
  std::vector<int> b(n, false), pre(n), I[2];
  for (int e = 0; e < n; e++) I[0].push_back(e);
  M1.build(I[1]), M2.build(I[1]);
  for (bool converged = false; !converged;) {
    pre.assign(n, false);
    std::vector L(1, std::vector<int>());
    for (int u : I[0])
      if (M1.oracle(u)) pre[u] = true, L[0].push_back(u);
    int m = 0;
    for (; L.back().size(); m += 2) {
      L.push_back({});
      for (int e : L[m]) {
        if ((converged = M2.oracle(e))) break;
        for (int f : I[1])
          if (!pre[f] && M2.oracle(f, e)) L[m + 1].push_back(f), pre[f] = true;
      }
      if (converged) break;
      L.push_back({});
      for (int e : L[m + 1])
        for (int f : I[0])
          if (!pre[f] && M1.oracle(e, f)) L[m + 2].push_back(f), pre[f] = true;
    }
    if (!converged) break;
    std::vector<std::vector<int>> L2(m + 1);
    for (int e : L[m])
      if (M2.oracle(e)) L2[m].push_back(e);
    for (int i = m; i; i -= 2) {
      for (int e : L[i - 1])
        for (int f : L2[i])
          if (M1.oracle(e, f)) {
            L2[i - 1].push_back(e);
            break;
          }
      for (int e : L[i - 2])
        for (int f : L2[i - 1])
          if (M2.oracle(f, e)) {
            L2[i - 2].push_back(e);
            break;
          }
    }
    pre.assign(n, -1);
    for (int e : L2[0])
      if (M1.oracle(e)) {
        bool isok = false;
        if (m) {
          std::vector<size_t> ei(m);
          for (int i = 0; e != -1;) {
            if (ei[i] == L2[i + 1].size()) e = pre[e], ei[i--] = 0;
            else if (int f = L2[i + 1][ei[i]++]; pre[f] == -1 && (i & 1 ? M1.oracle(e, f) : M2.oracle(f, e))) {
              if (pre[f] = e, e = f; ++i == m) {
                if (M2.oracle(e))
                  for (isok = true; e != -1; e = pre[e]) b[e] = !b[e];
                else e = pre[e], --i;
              }
            }
          }
        } else if (M2.oracle(e)) isok = true, b[e] = 1;
        if (isok) {
          converged = false, I[0].clear(), I[1].clear();
          for (int u = 0; u < n; u++) I[b[u]].push_back(u);
          M1.build(I[1]), M2.build(I[1]);
        }
      }
  }
  return I[1];
}

// I の基底を掃き出し、各行が I のどの要素の xor かを 64 bit の印で持つ。oracle(e) は I + e が独立か、
// oracle(e, f) は I - e + f が独立か (e が I にあり f が無い)。
struct XorMatroid {
  const std::vector<unsigned long long> *v;
  std::vector<int> pos;
  unsigned long long row[64] = {}, mask[64] = {};
  explicit XorMatroid(const std::vector<unsigned long long> &vec) : v(&vec), pos(vec.size(), -1) {}
  void build(const std::vector<int> &I) {
    std::fill(pos.begin(), pos.end(), -1);
    for (int b = 0; b < 64; ++b) row[b] = 0;
    for (int k = 0; k < (int)I.size(); ++k) {
      pos[I[k]] = k;
      unsigned long long x = (*v)[I[k]], m = 1ULL << k;
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
  // 従属なら I の中での印、独立なら ~0
  unsigned long long reduce(int f) const {
    unsigned long long x = (*v)[f], m = 0;
    while (x) {
      const int b = 63 - __builtin_clzll(x);
      if (!row[b]) return ~0ULL;
      x ^= row[b], m ^= mask[b];
    }
    return m;
  }
  bool oracle(int e) const { return reduce(e) == ~0ULL; }
  bool oracle(int e, int f) const {
    const unsigned long long m = reduce(f);
    return m == ~0ULL || (m >> pos[e] & 1);
  }
};

struct PartitionMatroid {
  std::vector<int> belong, R, cnt;
  PartitionMatroid(int m_, const std::vector<std::vector<int>> &parts, const std::vector<int> &R_) : belong(m_, -1), R(R_) {
    for (int i = parts.size(); i--;)
      for (int e : parts[i]) belong[e] = i;
  }
  void build(const std::vector<int> &I) {
    cnt = R;
    for (int e : I)
      if (belong[e] != -1) cnt[belong[e]]--;
  }
  inline bool oracle(int e) const { return belong[e] == -1 || cnt[belong[e]] > 0; }
  inline bool oracle(int e, int f) const { return oracle(f) || belong[e] == belong[f]; }
};
}  // namespace ref_mi

struct Solver {
  const vector<u64> &vec;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(const vector<u64> &vec, const vector<int> &color, const vector<int> &cap) : vec(vec), color(color), cap(cap) {}

  void run() {
    const int m = vec.size();
    ref_mi::XorMatroid x(vec);
    vector<vector<int>> parts(cap.size());
    for (int i = 0; i < m; ++i) parts[color[i]].push_back(i);
    ref_mi::PartitionMatroid p(m, parts, cap);
    ans = ref_mi::matroid_intersection(m, x, p);
  }

  vector<int> answer() const { return ans; }
};
