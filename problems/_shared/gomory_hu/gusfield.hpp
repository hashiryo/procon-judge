#pragma once
// Gomory-Hu 木を Gusfield の方法で作る部品。最大流の部分 (Engine) を差し替えて書き比べる。Engine は次を持つ。
//   Engine(int n, const vector<array<long long, 3>> &edges);  // 無向辺 {u, v, 容量}。網は 1 回だけ組む
//   long long flow(int s, int t);   // 流量を 0 に戻してから s から t へ最大流を流し、値を返す
//   bool side(int v) const;         // 直前の flow の最小カットで、v が s の側にあるか
// 手順は self-flow-gomory-hu の reference.hpp と同じ (処理済みの頂点も付け替え、t の親が s の側なら s を間に入れる)。
#include <array>
#include <vector>

namespace gomory_hu_shared {
using i64 = long long;
// 根を 0 とした木の親 p と、辺 (v, p[v]) の重み w を作る。p[0] は -1。
template <class Engine> void gusfield(int n, Engine &e, std::vector<int> &p, std::vector<i64> &w) {
  p.assign(n, 0), w.assign(n, 0);
  if (n > 0) p[0] = -1;
  for (int s = 1; s < n; ++s) {
    const int t = p[s];
    const i64 f = e.flow(s, t);
    w[s] = f;
    for (int i = 1; i < n; ++i)
      if (i != s && p[i] == t && e.side(i)) p[i] = s;
    if (t != 0 && e.side(p[t])) p[s] = p[t], p[t] = s, w[s] = w[t], w[t] = f;
  }
}
// 木の辺 {v, p[v], w[v]} の列にする。
template <class Engine> std::vector<std::array<i64, 3>> tree_edges(int n, Engine &e) {
  std::vector<int> p;
  std::vector<i64> w;
  gusfield(n, e, p, w);
  std::vector<std::array<i64, 3>> r;
  r.reserve(n > 0 ? n - 1 : 0);
  for (int v = 1; v < n; ++v) r.push_back({v, p[v], w[v]});
  return r;
}
}  // namespace gomory_hu_shared
