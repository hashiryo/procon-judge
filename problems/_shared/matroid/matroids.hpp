#pragma once
// マトロイドの交差の書き比べで使うマトロイド。どの型も次の 4 つを持ち、交差の算法の側がどれを使うかを選ぶ。
//   void build(const vector<int> &I)   独立集合 I に合わせて内部の構造を作り直す
//   bool can_add(int x)                 x が I に無いとき、I + x が独立か
//   bool can_exchange(int y, int x)     y が I にあり x が I に無いとき、I - y + x が独立か
//   void circuit(int x, F f)            I + x が独立でないとき、I + x の唯一の閉路に入る I の要素 y ごとに f(y) を呼ぶ
#include <algorithm>
#include <array>
#include <vector>

namespace mtr {
using ull = unsigned long long;

// グラフ的マトロイド。辺の集合が森であるものを独立とする。build で森を根付き木にし、各頂点の深さ、親への辺、
// オイラー順の入りと出を持つ。交換の判定は、y の下側の頂点の部分木に x の両端のちょうど一方が入るかで O(1)。
// 閉路の列挙は、x の両端から深いほうを親へたどる。
struct Graphic {
  int n;
  const std::vector<std::array<int, 2>> &e;
  std::vector<int> start, adj, comp, dep, pe, pv, tin, tout, stk, it;

  Graphic(int n, const std::vector<std::array<int, 2>> &edges)
      : n(n), e(edges), start(n + 1), comp(n), dep(n), pe(n), pv(n), tin(n), tout(n), it(n) {}

  void build(const std::vector<int> &I) {
    std::fill(start.begin(), start.end(), 0);
    for (int y : I) ++start[e[y][0] + 1], ++start[e[y][1] + 1];
    for (int v = 0; v < n; ++v) start[v + 1] += start[v];
    adj.resize(2 * I.size());
    for (int v = 0; v < n; ++v) it[v] = start[v];
    for (int y : I) adj[it[e[y][0]]++] = y, adj[it[e[y][1]]++] = y;
    std::fill(comp.begin(), comp.end(), -1);
    int t = 0;
    for (int r = 0; r < n; ++r) {
      if (comp[r] >= 0) continue;
      comp[r] = r, dep[r] = 0, pe[r] = -1, pv[r] = -1, tin[r] = t++, it[r] = start[r];
      stk.assign(1, r);
      while (!stk.empty()) {
        const int v = stk.back();
        if (it[v] == start[v + 1]) {
          tout[v] = t, stk.pop_back();
          continue;
        }
        const int y = adj[it[v]++];
        const int w = e[y][0] ^ e[y][1] ^ v;
        if (comp[w] >= 0) continue;
        comp[w] = r, dep[w] = dep[v] + 1, pe[w] = y, pv[w] = v, tin[w] = t++, it[w] = start[w];
        stk.push_back(w);
      }
    }
  }
  bool can_add(int x) const { return comp[e[x][0]] != comp[e[x][1]]; }
  bool can_exchange(int y, int x) const {
    if (can_add(x)) return true;
    const int c = dep[e[y][0]] > dep[e[y][1]] ? e[y][0] : e[y][1];
    auto inside = [&](int w) { return tin[c] <= tin[w] && tin[w] < tout[c]; };
    return inside(e[x][0]) != inside(e[x][1]);
  }
  template <class F> void circuit(int x, F &&f) const {
    int u = e[x][0], v = e[x][1];
    while (u != v) {
      if (dep[u] < dep[v]) std::swap(u, v);
      f(pe[u]);
      u = pv[u];
    }
  }
};

// 分割マトロイド。色 c の要素を cap[c] 個まで選べる。閉路は、x と同じ色で I に入っている要素の全部。
struct Partition {
  const std::vector<int> &color, &cap;
  std::vector<int> cnt, start, mem;

  Partition(const std::vector<int> &color, const std::vector<int> &cap)
      : color(color), cap(cap), cnt(cap.size()), start(cap.size() + 1) {}

  void build(const std::vector<int> &I) {
    std::fill(cnt.begin(), cnt.end(), 0);
    for (int y : I) ++cnt[color[y]];
    start[0] = 0;
    for (size_t c = 0; c < cnt.size(); ++c) start[c + 1] = start[c] + cnt[c];
    mem.resize(I.size());
    for (int y : I) mem[start[color[y] + 1] - cnt[color[y]]--] = y;
    for (size_t c = 0; c < cnt.size(); ++c) cnt[c] = start[c + 1] - start[c];
  }
  bool can_add(int x) const { return cnt[color[x]] < cap[color[x]]; }
  bool can_exchange(int y, int x) const { return can_add(x) || color[y] == color[x]; }
  template <class F> void circuit(int x, F &&f) const {
    const int c = color[x];
    for (int k = start[c]; k < start[c + 1]; ++k) f(mem[k]);
  }
};

// GF(2) の線形マトロイド (64 bit のベクトル)。build で I の基底を掃き出し、各基底の行が I のどの要素の xor かを
// 64 bit の印で持つ (|I| <= 64)。x を基底で消して 0 になれば従属で、残った印が閉路。消した結果は build ごとに覚える。
struct Xor64 {
  const std::vector<ull> &v;
  std::vector<int> idx;                  // I の要素の、I の中での番号
  std::vector<int> elem;                 // I の中での番号から要素へ
  ull row[64] = {}, mask[64] = {};     // ピボットの bit ごとの行と、その行を作る I の要素の印
  int rank = 0;
  std::vector<ull> memo;       // x を消したあとの印 (従属なら)。独立なら ~0
  std::vector<std::uint32_t> stamp;
  std::uint32_t now = 0;

  explicit Xor64(const std::vector<ull> &vec) : v(vec), idx(vec.size(), -1), memo(vec.size()), stamp(vec.size()) {}

  void build(const std::vector<int> &I) {
    for (int y : elem) idx[y] = -1;
    elem = I;
    rank = 0;
    for (int b = 0; b < 64; ++b) row[b] = 0;
    for (int k = 0; k < (int)I.size(); ++k) {
      idx[I[k]] = k;
      ull x = v[I[k]], m = ull(1) << k;
      while (x) {
        const int b = 63 - __builtin_clzll(x);
        if (!row[b]) {
          row[b] = x, mask[b] = m, ++rank;
          break;
        }
        x ^= row[b], m ^= mask[b];
      }
    }
    ++now;
  }
  // x を消したあとの印。独立なら ~0
  ull reduce(int x) {
    if (stamp[x] == now) return memo[x];
    stamp[x] = now;
    ull a = v[x], m = 0;
    while (a) {
      const int b = 63 - __builtin_clzll(a);
      if (!row[b]) return memo[x] = ~ull(0);
      a ^= row[b], m ^= mask[b];
    }
    return memo[x] = m;
  }
  bool can_add(int x) { return reduce(x) == ~ull(0); }
  bool can_exchange(int y, int x) {
    const ull m = reduce(x);
    return m == ~ull(0) || (m >> idx[y] & 1);
  }
  template <class F> void circuit(int x, F &&f) {
    for (ull m = reduce(x); m; m &= m - 1) f(elem[__builtin_ctzll(m)]);
  }
};
}  // namespace mtr
