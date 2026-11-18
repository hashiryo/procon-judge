#pragma once
// 参照実装。今の Library の mylib/optimization/matroid_intersection.hpp (Library 459f66b81) の matroid_intersection と
// GraphicMatroid、PartitionMatroid を写したもの。Library を直しても期待出力が変わらないように、include せずに持つ。
// 交換の判定 oracle(e, f) を 2 引数で呼び、最短の増加路を層ごとにまとめて探す (Cunningham)。
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

class GraphicMatroid {
  int n;
  std::vector<std::array<int, 2>> es;
  std::vector<int> g, pos, comp, in, out;
  inline bool is_ancestor(int u, int v) const { return in[u] <= in[v] && in[v] < out[u]; }

 public:
  GraphicMatroid(int n_) : n(n_), comp(n), in(n), out(n) {}
  int add_edge(int u, int v) { return es.push_back({u, v}), es.size() - 1; }
  void build(const std::vector<int> &I) {
    in.assign(n, -1), g.resize(I.size() * 2), pos.assign(n + 1, 0);
    for (int e : I) {
      auto [u, v] = es[e];
      ++pos[u], ++pos[v];
    }
    for (int i = 0; i < n; ++i) pos[i + 1] += pos[i];
    for (int e : I) {
      auto [u, v] = es[e];
      g[--pos[u]] = v, g[--pos[v]] = u;
    }
    std::vector<int> ei(pos.begin(), pos.begin() + n), pre(n, -1);
    for (int u = 0, t = 0, p; u < n; u++)
      if (in[u] == -1)
        for (in[comp[u] = p = u] = t++; p >= 0;) {
          if (ei[p] == pos[p + 1]) out[p] = t, p = pre[p];
          else if (int v = g[ei[p]++]; in[v] == -1) comp[v] = comp[u], pre[v] = p, in[p = v] = t++;
        }
  }
  inline bool oracle(int e) const { return comp[es[e][0]] != comp[es[e][1]]; }
  inline bool oracle(int e, int f) const {
    if (oracle(f)) return true;
    return e = es[e][in[es[e][0]] < in[es[e][1]]], is_ancestor(e, es[f][0]) != is_ancestor(e, es[f][1]);
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
  int n;
  const vector<array<int, 2>> &edges;
  const vector<int> &color, &cap;
  vector<int> ans;

  Solver(int n, const vector<array<int, 2>> &edges, const vector<int> &color, const vector<int> &cap)
      : n(n), edges(edges), color(color), cap(cap) {}

  void run() {
    const int m = edges.size();
    ref_mi::GraphicMatroid g(n);
    for (auto &e : edges) g.add_edge(e[0], e[1]);
    vector<vector<int>> parts(cap.size());
    for (int i = 0; i < m; ++i) parts[color[i]].push_back(i);
    ref_mi::PartitionMatroid p(m, parts, cap);
    ans = ref_mi::matroid_intersection(m, g, p);
  }

  vector<int> answer() const { return ans; }
};
