#pragma once
// s から t への道の i 番目の頂点。倍加で LCA と k 番目の祖先を出す。
// 道の長さ d = dep[s] + dep[t] - 2 dep[lca]。i <= dep[s] - dep[lca] なら s の i 番目の祖先、
// そうでなければ t の (d - i) 番目の祖先。
#include <vector>

struct Solver {
  int n, LOG;
  std::vector<int> head, nxt, to;
  std::vector<std::vector<int>> up;
  std::vector<int> dep, ans;
  std::vector<int> qs, qt, qk;

  Solver(int n_, const std::vector<int> &a, const std::vector<int> &b,
         std::vector<int> s, std::vector<int> t, std::vector<int> k)
      : n(n_), qs(std::move(s)), qt(std::move(t)), qk(std::move(k)) {
    const int m = n_ - 1;
    head.assign(n, -1);
    nxt.resize(2 * m);
    to.resize(2 * m);
    for (int i = 0; i < m; ++i) {
      to[2 * i] = b[i], nxt[2 * i] = head[a[i]], head[a[i]] = 2 * i;
      to[2 * i + 1] = a[i], nxt[2 * i + 1] = head[b[i]], head[b[i]] = 2 * i + 1;
    }
    LOG = 1;
    while ((1 << LOG) < n) ++LOG;
  }

  int jump(int v, int k) const {
    for (int j = 0; k; ++j, k >>= 1)
      if (k & 1) v = up[j][v];
    return v;
  }

  void run() {
    up.assign(LOG, std::vector<int>(n, 0));
    dep.assign(n, 0);
    std::vector<int> order;
    order.reserve(n);
    order.push_back(0);
    std::vector<signed char> seen(n, 0);
    seen[0] = 1;
    for (size_t i = 0; i < order.size(); ++i) {
      const int v = order[i];
      for (int e = head[v]; e != -1; e = nxt[e]) {
        const int u = to[e];
        if (seen[u]) continue;
        seen[u] = 1;
        dep[u] = dep[v] + 1;
        up[0][u] = v;
        order.push_back(u);
      }
    }
    for (int j = 1; j < LOG; ++j)
      for (int v = 0; v < n; ++v) up[j][v] = up[j - 1][up[j - 1][v]];
    ans.resize(qs.size());
    for (size_t qi = 0; qi < qs.size(); ++qi) {
      int s = qs[qi], t = qt[qi];
      const int k = qk[qi];
      // lca
      int x = s, y = t;
      if (dep[x] < dep[y]) std::swap(x, y);
      x = jump(x, dep[x] - dep[y]);
      int l;
      if (x == y) {
        l = x;
      } else {
        for (int j = LOG - 1; j >= 0; --j)
          if (up[j][x] != up[j][y]) x = up[j][x], y = up[j][y];
        l = up[0][x];
      }
      const int d = dep[s] + dep[t] - 2 * dep[l];
      if (k > d) {
        ans[qi] = -1;
      } else if (k <= dep[s] - dep[l]) {
        ans[qi] = jump(s, k);
      } else {
        ans[qi] = jump(t, d - k);
      }
    }
  }

  const std::vector<int> &answer() const { return ans; }
};
