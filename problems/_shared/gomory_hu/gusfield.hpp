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
// gusfield と同じだが、頂点を処理する順を、同じ親を持つ未処理の頂点が続くように選ぶ (Engine が t ごとに覚えておく
// ものを使い回せるように)。Gusfield の方法はどの順で頂点を処理しても正しく、順を処理の途中で決めても、決まった順で
// 回したのと同じになる (各段の手順は、それより後に処理する頂点によらない)。今の親 t の未処理の子を先に処理し尽くし、
// 途中で子を持った頂点は後に回す。
template <class Engine> void gusfield_grouped(int n, Engine &e, std::vector<int> &p, std::vector<i64> &w) {
  p.assign(n, 0), w.assign(n, 0);
  if (n <= 1) {
    if (n == 1) p[0] = -1;
    return;
  }
  p[0] = -1;
  // 未処理の頂点を、親ごとの双方向リストに持つ。
  std::vector<int> head(n, -1), nxt(n, -1), prv(n, -1), pend;
  std::vector<char> done(n, 0);
  auto link = [&](int v, int t) {
    prv[v] = -1, nxt[v] = head[t];
    if (head[t] >= 0) prv[head[t]] = v;
    head[t] = v;
  };
  auto unlink = [&](int v, int t) {
    if (prv[v] >= 0) nxt[prv[v]] = nxt[v];
    else head[t] = nxt[v];
    if (nxt[v] >= 0) prv[nxt[v]] = prv[v];
  };
  for (int v = n - 1; v >= 1; --v) link(v, 0);
  pend.push_back(0);
  while (!pend.empty()) {
    const int t = pend.back();
    if (head[t] < 0) {
      pend.pop_back();
      continue;
    }
    const int s = head[t];
    unlink(s, t), done[s] = 1;
    const i64 f = e.flow(s, t);
    w[s] = f;
    bool got = false;
    for (int i = 1; i < n; ++i)
      if (i != s && p[i] == t && e.side(i)) {
        p[i] = s;
        if (!done[i]) unlink(i, t), link(i, s), got = true;
      }
    if (t != 0 && e.side(p[t])) p[s] = p[t], p[t] = s, w[s] = w[t], w[t] = f;
    if (got) pend.insert(pend.end() - 1, s);  // t の子を先に処理し尽くす
  }
}
// 木の辺 {v, p[v], w[v]} の列にする。
template <class Engine, bool GROUPED = false> std::vector<std::array<i64, 3>> tree_edges(int n, Engine &e) {
  std::vector<int> p;
  std::vector<i64> w;
  if constexpr (GROUPED) gusfield_grouped(n, e, p, w);
  else gusfield(n, e, p, w);
  std::vector<std::array<i64, 3>> r;
  r.reserve(n > 0 ? n - 1 : 0);
  for (int v = 1; v < n; ++v) r.push_back({v, p[v], w[v]});
  return r;
}
}  // namespace gomory_hu_shared
