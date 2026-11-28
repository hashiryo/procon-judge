#pragma once
// 部分木加算付き Link-Cut Tree の速い版 (chaihf の 278245 の形)。
// 節は道の和 sum と頂点数 size、仮想の子の和 sum_ と頂点数 size_、道の遅延 add、反転 rev を持つ。
// add は「その節の補助木の全体 (道と仮想の子)」に掛かる遅延で、回転のときに
//   x.add = a + b、p.add = -b、切り離す子に +b
// と付け替える (push しない)。access は splay のあと右の子を仮想へ移し、道の親を splay して
// 右の子に積み、1 回回す (最後の splay を省く)。初期木は親の向きから O(N) で組む。
// 記録は algo-notes の notes/heavy-algorithms.md。
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

class LctFast {
 public:
  using i64 = long long;
  LctFast() = default;
  // a: 頂点の初期値。edges: 初期の木の辺 (n - 1 本)。0 始まりの頂点番号。
  LctFast(const std::vector<i64>& a, const std::vector<std::pair<int, int>>& edges) { build(a, edges); }

  void cut(int u, int v) {
    u += 1, v += 1;
    access(u);
    node[u].rev ^= 1;
    access(v);
    int w = node[v].ch[0];
    node[w].p = 0, node[w].k = -1;
    node[w].add += node[v].add;
    node[v].ch[0] = 0;
  }
  void link(int u, int v) {
    u += 1, v += 1;
    access(u);
    node[u].rev ^= 1;
    access(v);
    node[u].p = v;
    node[u].add -= node[v].add;
    node[u].sum -= node[v].add * node[u].size;
    node[v].sum_ += node[u].sum;
    node[v].size_ += node[u].size;
  }
  // 辺 (v, p) について、p を親とした v の部分木に x を足す。
  void apply_subtree(int v, int p, i64 x) {
    v += 1, p += 1;
    access(v);
    node[v].rev ^= 1;
    access(p);
    node[v].add += x;
    node[v].sum += x * node[v].size;
  }
  // 辺 (v, p) について、p を親とした v の部分木の和。
  i64 prod_subtree(int v, int p) {
    v += 1, p += 1;
    access(v);
    node[v].rev ^= 1;
    access(p);
    return node[v].sum + node[v].size * node[p].add;
  }

 private:
  struct Node {
    i64 sum = 0, sum_ = 0, add = 0;
    int size = 0, size_ = 0, p = 0, ch[2] = {0, 0};
    signed char rev = 0, k = -1;
  };
  std::vector<Node> node;
  int n = 0;

  void build(const std::vector<i64>& a, const std::vector<std::pair<int, int>>& edges) {
    n = (int)a.size();
    node.assign(n + 1, Node{});
    for (int i = 0; i < n; ++i) node[i + 1].sum_ = a[i], node[i + 1].k = -1;
    std::vector<std::vector<int>> adj(n);
    for (auto [u, v] : edges) adj[u].push_back(v), adj[v].push_back(u);
    std::vector<int> par(n, -1), order;
    order.reserve(n);
    order.push_back(0);
    for (std::size_t i = 0; i < order.size(); ++i) {
      const int x = order[i];
      for (int y : adj[x])
        if (y != par[x]) par[y] = x, order.push_back(y);
    }
    for (int i = n - 1; i >= 0; --i) {
      const int v = order[i];
      node[v + 1].size = node[v + 1].size_ + 1;
      node[v + 1].sum = node[v + 1].sum_;
      if (const int p = par[v]; p >= 0) {
        node[v + 1].p = p + 1;
        node[p + 1].sum_ += node[v + 1].sum;
        node[p + 1].size_ += node[v + 1].size;
      }
    }
  }
  void pushdown(int x) {
    if (node[x].rev) {
      std::swap(node[x].ch[0], node[x].ch[1]);
      const int l = node[x].ch[0], r = node[x].ch[1];
      node[l].k = 0, node[r].k = 1;
      node[l].rev ^= 1, node[r].rev ^= 1;
      node[x].rev = 0;
    }
  }
  void maintain(int x) {
    Node& a = node[x];
    a.size = node[a.ch[0]].size + 1 + node[a.ch[1]].size + a.size_;
    a.sum = node[a.ch[0]].sum + node[a.ch[1]].sum + a.sum_ + a.size * a.add;
  }
  void rotateup(int x) {
    const int p = node[x].p, g = node[p].p, k = node[x].k, t = node[p].k;
    const i64 a = node[p].add, b = node[x].add;
    node[x].add = a + b;
    node[p].add = -b;
    const int c = node[x].ch[k ^ 1];
    node[c].add += b, node[c].sum += b * node[c].size;
    node[c].p = p, node[c].k = k;
    node[p].ch[k] = c;
    node[p].p = x, node[p].k = k ^ 1;
    node[x].ch[k ^ 1] = p;
    node[x].p = g, node[x].k = t;
    if (t != -1) node[g].ch[t] = x;
    maintain(p);
  }
  bool is_root(int x) const { return node[x].k == -1; }
  void splay(int x) {
    pushdown(x);
    while (!is_root(x)) {
      const int p = node[x].p;
      if (is_root(p)) {
        pushdown(p), pushdown(x);
        rotateup(x);
      } else {
        const int g = node[p].p;
        pushdown(g), pushdown(p), pushdown(x);
        if (node[x].k == node[p].k) rotateup(p), rotateup(x);
        else rotateup(x), rotateup(x);
      }
    }
  }
  void access(int x) {
    splay(x);
    int r = node[x].ch[1];
    node[r].k = -1;
    node[x].sum_ += node[r].sum;
    node[x].size_ += node[r].size;
    node[x].ch[1] = 0;
    maintain(x);
    while (const int p = node[x].p) {
      splay(p);
      r = node[p].ch[1];
      node[r].k = -1;
      node[p].sum_ += node[r].sum;
      node[p].size_ += node[r].size;
      node[p].sum_ -= node[x].sum;
      node[p].size_ -= node[x].size;
      node[p].ch[1] = x;
      node[x].k = 1;
      rotateup(x);
      maintain(x);
    }
  }
};
