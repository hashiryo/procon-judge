#pragma once
// マトロイドの交差の試作 (2026-11-18 に本人と決めた案 C の形)。マトロイドの型は次を持つ。
//   int size()                          要素の数
//   void build(const vector<int> &I)    独立集合 I に合わせて中の構造を作り直す
//   bool can_add(int x)                 x が I に無いとき、I + x が独立か
// さらに、次の 2 つのうち書きやすいほう (両方でもよい) を持つ。
//   bool can_exchange(int y, int x)     y が I にあり x が I に無いとき、I - y + x が独立か
//   void circuit(int x, F f)            I + x が独立でないとき、I + x の唯一の閉路に入る I の要素 y ごとに f(y) を呼ぶ
// 足りないほうは Adapter が補う。can_exchange が無ければ、x の閉路を build ごとに覚えて y が入っているかを見る。circuit が
// 無ければ、I の要素 y を全部 can_exchange(y, x) で調べる。
//   matroid_intersection(a, b)             重みなし。2 引数の判定で層を作り、層に沿った最短の増加路を次々に探す
//                                          (Cunningham の方法)。can_exchange を持たない型は、x を固定した問いだけを受ける
//                                          側 (M2) に回す
//   weighted_matroid_intersection(a, b, w) 重み付き。閉路の列挙で交換グラフを作り、要素の長さ (I に無い x は -w(x)、
//                                          I にある y は w(y)) の和と辺の数の組で最小の路を Bellman-Ford (待ち行列) で
//                                          求めて 1 本ずつ増やす。返り値の best[k] は要素数 k の重みの和の最大
#include <algorithm>
#include <array>
#include <concepts>
#include <limits>
#include <vector>

namespace flex {
using i64 = long long;

template <class M>
concept HasExchange = requires(M &m, int y, int x) {
  { m.can_exchange(y, x) } -> std::convertible_to<bool>;
};
template <class M>
concept HasCircuit = requires(M &m, int x) { m.circuit(x, [](int) {}); };

template <class M> struct Adapter {
  static_assert(HasExchange<M> || HasCircuit<M>, "matroid needs can_exchange or circuit");
  M &m;
  const std::vector<int> *I = nullptr;
  // circuit だけの型のための覚え書き。cache[x] は stamp[x] == ver のとき、今の I での x の閉路。印 mark[y] == tag は、
  // y が owner の閉路に入っていること
  std::vector<std::vector<int>> cache;
  std::vector<unsigned> stamp, mark;
  unsigned ver = 1, tag = 0;
  int owner = -1;

  explicit Adapter(M &m) : m(m) {
    if constexpr (!HasExchange<M>) cache.resize(m.size()), stamp.assign(m.size(), 0), mark.assign(m.size(), 0);
  }
  void build(const std::vector<int> &J) {
    I = &J, m.build(J);
    if constexpr (!HasExchange<M>) ++ver, owner = -1;
  }
  bool can_add(int x) { return m.can_add(x); }
  const std::vector<int> &circ(int x) {
    if (stamp[x] != ver) {
      stamp[x] = ver;
      cache[x].clear();
      m.circuit(x, [&](int y) { cache[x].push_back(y); });
    }
    return cache[x];
  }
  bool can_exchange(int y, int x) {
    if constexpr (HasExchange<M>) return m.can_exchange(y, x);
    else {
      if (m.can_add(x)) return true;
      if (owner != x) {
        owner = x, ++tag;
        for (int z : circ(x)) mark[z] = tag;
      }
      return mark[y] == tag;
    }
  }
  template <class F> void circuit(int x, F &&f) {
    if constexpr (HasCircuit<M>) m.circuit(x, f);
    else
      for (int y : *I)
        if (m.can_exchange(y, x)) f(y);
  }
};

namespace internal {
// 重みなし。b は x を固定した問い (can_exchange(・, x)) だけを受ける。a への問いも、層を作るところは x を固定する順に回す
template <class A, class B> std::vector<int> intersect(A &a, B &b) {
  const int n = a.m.size();
  std::vector<char> in(n), dead(n);
  std::vector<int> I, rest_out, rest_in, it, stk;
  auto rebuild = [&] {
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : stk)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
  };
  rebuild();
  std::vector<std::vector<int>> L;
  std::vector<char> good(n);
  while (true) {
    // 層を作る。偶数の層は I に無い要素、奇数の層は I の要素
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
      for (size_t k = 0; k < rest_out.size();) {
        const int x = rest_out[k];
        bool hit = false;
        for (int y : L[d + 1])
          if (a.can_exchange(y, x)) {
            hit = true;
            break;
          }
        if (hit) L[d + 2].push_back(x), rest_out[k] = rest_out.back(), rest_out.pop_back();
        else ++k;
      }
    }
    if (top < 0) break;
    // 初めの I で終点まで届かない要素を、後ろの層から除く。奇数の層 (I の要素 y) は、次の層の x を固定して印を付ける
    {
      auto &T = L[top];
      T.erase(std::remove_if(T.begin(), T.end(), [&](int x) { return !b.can_add(x); }), T.end());
      for (int d = top - 1; d >= 0; --d) {
        auto &C = L[d];
        const auto &nx = L[d + 1];
        if (d % 2 == 0) {
          C.erase(std::remove_if(C.begin(), C.end(),
                                 [&](int u) {
                                   for (int v : nx)
                                     if (b.can_exchange(v, u)) return false;
                                   return true;
                                 }),
                  C.end());
        } else {
          for (int y : C) good[y] = 0;
          for (int x : nx)
            for (int y : C)
              if (!good[y] && a.can_exchange(y, x)) good[y] = 1;
          C.erase(std::remove_if(C.begin(), C.end(), [&](int y) { return !good[y]; }), C.end());
        }
      }
    }
    // 層に沿った最短の増加路を、今の I での判定で次々に探す
    std::fill(dead.begin(), dead.end(), 0);
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
    stk.clear();
    if (!any) break;
  }
  return I;
}
}  // namespace internal

template <class M1, class M2> std::vector<int> matroid_intersection(M1 &m1, M2 &m2) {
  Adapter<M1> a(m1);
  Adapter<M2> b(m2);
  // can_exchange を持たない型は、x を固定した問いだけを受ける側に回す
  if constexpr (!HasExchange<M1> && HasExchange<M2>) return internal::intersect(b, a);
  else return internal::intersect(a, b);
}

struct Weighted {
  std::vector<i64> best;  // best[k] は要素数 k の重みの和の最大
  std::vector<int> set;   // 要素数が最大で重みの和が best.back() の集合
};

template <class M1, class M2> Weighted weighted_matroid_intersection(M1 &m1, M2 &m2, const std::vector<i64> &w) {
  Adapter<M1> a(m1);
  Adapter<M2> b(m2);
  const int n = m1.size();
  const i64 INF = std::numeric_limits<i64>::max() / 4;
  std::vector<char> in(n), src(n), snk(n), inq(n);
  std::vector<int> I, hop(n), par(n), start(n + 1), to, path, q;
  std::vector<i64> d(n);
  std::vector<std::array<int, 2>> es;
  Weighted res;
  res.best.push_back(0);
  a.build(I), b.build(I);
  while (true) {
    // 交換グラフ: y ∈ C1(x) なら y → x、y ∈ C2(x) なら x → y
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
    std::fill(d.begin(), d.end(), INF);
    q.assign(n + 1, 0);
    int qh = 0, qt = 0;
    for (int x = 0; x < n; ++x)
      if (src[x]) d[x] = -w[x], hop[x] = 0, par[x] = -1, inq[x] = 1, q[qt++] = x;
    while (qh != qt) {
      const int u = q[qh];
      qh = qh == n ? 0 : qh + 1;
      inq[u] = 0;
      for (int k = start[u]; k < start[u + 1]; ++k) {
        const int v = to[k];
        if (src[v]) continue;
        const i64 nd = d[u] + (in[v] ? w[v] : -w[v]);
        const int nh = hop[u] + 1;
        if (nd < d[v] || (nd == d[v] && nh < hop[v])) {
          d[v] = nd, hop[v] = nh, par[v] = u;
          if (!inq[v]) inq[v] = 1, q[qt] = v, qt = qt == n ? 0 : qt + 1;
        }
      }
    }
    int t = -1;
    for (int x = 0; x < n; ++x)
      if (snk[x] && d[x] < INF && (t < 0 || d[x] < d[t] || (d[x] == d[t] && hop[x] < hop[t]))) t = x;
    if (t < 0) break;
    path.clear();
    for (int v = t; v >= 0; v = par[v]) path.push_back(v);
    for (int v : path) in[v] ^= 1;
    I.erase(std::remove_if(I.begin(), I.end(), [&](int y) { return !in[y]; }), I.end());
    for (int v : path)
      if (in[v]) I.push_back(v);
    a.build(I), b.build(I);
    res.best.push_back(res.best.back() - d[t]);
  }
  res.set = I;
  return res;
}

// 書き比べのための包み。片方の操作だけを見せる
template <class M> struct OnlyExchange {
  M &m;
  int size() const { return m.size(); }
  void build(const std::vector<int> &I) { m.build(I); }
  bool can_add(int x) { return m.can_add(x); }
  bool can_exchange(int y, int x) { return m.can_exchange(y, x); }
};
template <class M> struct OnlyCircuit {
  M &m;
  int size() const { return m.size(); }
  void build(const std::vector<int> &I) { m.build(I); }
  bool can_add(int x) { return m.can_add(x); }
  template <class F> void circuit(int x, F &&f) { m.circuit(x, f); }
};
}  // namespace flex
