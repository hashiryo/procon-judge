#pragma once
// マトロイドの交差 (重みなし) の書き比べ。マトロイドの型に求める操作の形 (matroids.hpp の 4 つのうちどれを使うか) を
// 変えた 3 つの算法を置く。どれも、M1 に足せる要素から M2 に足せる要素への最短の増加路を探す。
//   intersect_pair     2 引数の交換の判定 (can_exchange) だけを使う。層を BFS で作り、層に沿った最短の増加路を
//                      DFS で次々に探して、そのたびに build する (Cunningham の方法)。今の Library と同じ考え方
//   intersect_circuit  閉路の列挙 (circuit) で交換グラフを毎回作り、BFS で最短の増加路を 1 本だけ探す
//                      (hitonanode の MatroidIntersection と同じ考え方)。初めに両方へ足せる要素を貪欲に足す
//   intersect_hybrid   層は閉路の列挙で作った交換グラフの BFS で作り、増加路は intersect_pair と同じ DFS で探す
#include <algorithm>
#include <array>
#include <vector>

namespace mi_algo {
// 層に沿った最短の増加路を、今の I での交換の判定で次々に探す。L[0] が始点の層で、L[top] に終点 (M2 に足せる要素) がある。
// 見つけるたびに in を反転して rebuild を呼ぶ。1 本でも見つけたら true
template <class M1, class M2, class R>
bool augment_layers(M1 &a, M2 &b, std::vector<std::vector<int>> &L, int top, std::vector<char> &in, std::vector<char> &dead,
                    std::vector<int> &it, std::vector<int> &stk, R &&rebuild) {
  std::fill(dead.begin(), dead.end(), 0);
  // 初めの I で終点まで届かない要素を、後ろの層から除く (今の Library の L2 と同じ)
  {
    auto &T = L[top];
    T.erase(std::remove_if(T.begin(), T.end(), [&](int x) { return !b.can_add(x); }), T.end());
    for (int d = top - 1; d >= 0; --d) {
      auto &C = L[d];
      const auto &nx = L[d + 1];
      C.erase(std::remove_if(C.begin(), C.end(),
                             [&](int u) {
                               for (int v : nx)
                                 if (d % 2 == 0 ? b.can_exchange(v, u) : a.can_exchange(u, v)) return false;
                               return true;
                             }),
              C.end());
    }
  }
  it.assign(top + 1, 0);
  bool any = false;
  for (int s : L[0]) {
    if (dead[s] || in[s] || !a.can_add(s)) continue;
    stk.assign(1, s);
    it[0] = 0;
    while (!stk.empty()) {
      const int d = (int)stk.size() - 1, u = stk.back();
      if (d == top) {
        if (b.can_add(u)) {
          for (int v : stk) in[v] ^= 1, dead[v] = 1;
          rebuild();
          any = true;
          break;
        }
        dead[u] = 1, stk.pop_back();
        continue;
      }
      const auto &nx = L[d + 1];
      bool moved = false;
      while (it[d] < (int)nx.size()) {
        const int v = nx[it[d]++];
        if (dead[v]) continue;
        if (d % 2 == 0 ? b.can_exchange(v, u) : a.can_exchange(u, v)) {
          stk.push_back(v), it[d + 1] = 0, moved = true;
          break;
        }
      }
      if (!moved) dead[u] = 1, stk.pop_back();
    }
  }
  return any;
}

template <class M1, class M2> std::vector<int> intersect_pair(int n, M1 &a, M2 &b) {
  std::vector<char> in(n), dead(n);
  std::vector<int> I, rest_out, rest_in, it, stk;
  // 増加路で入れ替わった要素だけを I から抜き差しする (I 全体を n から作り直さない)
  auto rebuild = [&] {
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : stk)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
  };
  rebuild();
  std::vector<std::vector<int>> L;
  while (true) {
    L.assign(1, {});
    rest_out.clear();
    for (int x = 0; x < n; ++x)
      if (!in[x]) (a.can_add(x) ? L[0] : rest_out).push_back(x);
    rest_in = I;
    int top = -1;
    for (int d = 0; !L[d].empty(); d += 2) {
      bool sink = false;
      for (int x : L[d])
        if (b.can_add(x)) {
          sink = true;
          break;
        }
      if (sink) {
        top = d;
        break;
      }
      L.emplace_back(), L.emplace_back();
      for (int x : L[d])
        for (size_t k = 0; k < rest_in.size();) {
          const int y = rest_in[k];
          if (b.can_exchange(y, x)) L[d + 1].push_back(y), rest_in[k] = rest_in.back(), rest_in.pop_back();
          else ++k;
        }
      for (int y : L[d + 1])
        for (size_t k = 0; k < rest_out.size();) {
          const int x = rest_out[k];
          if (a.can_exchange(y, x)) L[d + 2].push_back(x), rest_out[k] = rest_out.back(), rest_out.pop_back();
          else ++k;
        }
    }
    if (top < 0 || !augment_layers(a, b, L, top, in, dead, it, stk, rebuild)) break;
  }
  return I;
}

// 交換グラフを閉路の列挙で作る。辺は、y ∈ C1(x) なら y → x、y ∈ C2(x) なら x → y。src と snk は M1、M2 に足せる要素
template <class M1, class M2>
void exchange_graph(int n, M1 &a, M2 &b, const std::vector<char> &in, std::vector<char> &src, std::vector<char> &snk,
                    std::vector<int> &start, std::vector<int> &to, std::vector<std::array<int, 2>> &es) {
  es.clear();
  for (int x = 0; x < n; ++x) {
    src[x] = snk[x] = 0;
    if (in[x]) continue;
    if (a.can_add(x)) src[x] = 1;
    else a.circuit(x, [&](int y) { es.push_back({y, x}); });
    if (b.can_add(x)) snk[x] = 1;
    else b.circuit(x, [&](int y) { es.push_back({x, y}); });
  }
  std::fill(start.begin(), start.end(), 0);
  for (auto &[u, v] : es) ++start[u + 1];
  for (int u = 0; u < n; ++u) start[u + 1] += start[u];
  to.resize(es.size());
  for (auto &[u, v] : es) to[start[u]++] = v;
  for (int u = n; u > 0; --u) start[u] = start[u - 1];
  start[0] = 0;
}

template <class M1, class M2> std::vector<int> intersect_circuit(int n, M1 &a, M2 &b) {
  std::vector<char> in(n), src(n), snk(n);
  std::vector<int> I, start(n + 1), to, dist(n), par(n), q;
  std::vector<std::array<int, 2>> es;
  a.build(I), b.build(I);
  for (int x = 0; x < n; ++x)
    if (a.can_add(x) && b.can_add(x)) in[x] = 1, I.push_back(x), a.build(I), b.build(I);
  while (true) {
    exchange_graph(n, a, b, in, src, snk, start, to, es);
    std::fill(dist.begin(), dist.end(), -1);
    q.clear();
    for (int x = 0; x < n; ++x)
      if (src[x]) dist[x] = 0, par[x] = -1, q.push_back(x);
    int t = -1;
    for (size_t h = 0; h < q.size() && t < 0; ++h) {
      const int u = q[h];
      if (snk[u]) {
        t = u;
        break;
      }
      for (int k = start[u]; k < start[u + 1]; ++k) {
        const int v = to[k];
        if (dist[v] < 0) dist[v] = dist[u] + 1, par[v] = u, q.push_back(v);
      }
    }
    if (t < 0) break;
    for (int v = t; v >= 0; v = par[v]) in[v] ^= 1;
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v = t; v >= 0; v = par[v])
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
  }
  return I;
}

template <class M1, class M2> std::vector<int> intersect_hybrid(int n, M1 &a, M2 &b) {
  std::vector<char> in(n), src(n), snk(n), dead(n);
  std::vector<int> I, start(n + 1), to, lv(n), it, stk;
  std::vector<std::array<int, 2>> es;
  auto rebuild = [&] {
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : stk)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
  };
  rebuild();
  std::vector<std::vector<int>> L;
  while (true) {
    exchange_graph(n, a, b, in, src, snk, start, to, es);
    std::fill(lv.begin(), lv.end(), -1);
    L.assign(1, {});
    for (int x = 0; x < n; ++x)
      if (src[x]) lv[x] = 0, L[0].push_back(x);
    int top = -1;
    for (int d = 0; !L[d].empty(); ++d) {
      if (d % 2 == 0) {
        bool sink = false;
        for (int x : L[d])
          if (snk[x]) {
            sink = true;
            break;
          }
        if (sink) {
          top = d;
          break;
        }
      }
      L.emplace_back();
      for (int u : L[d])
        for (int k = start[u]; k < start[u + 1]; ++k)
          if (const int v = to[k]; lv[v] < 0) lv[v] = d + 1, L[d + 1].push_back(v);
    }
    if (top < 0 || !augment_layers(a, b, L, top, in, dead, it, stk, rebuild)) break;
  }
  return I;
}
}  // namespace mi_algo
