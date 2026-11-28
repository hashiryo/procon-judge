#pragma once
#include <array>

#include "pj.hpp"

// incremental Schreier–Sims (Knuth の Algorithm A/B) に、入力だけで S_n を証明する近道を足した版。
// 2-サイクルがちょうど 1 つで他のサイクルがすべて奇数長の生成元から転置を作り、その共役で
// できる辺が全点を連結なら S_n として n! を返す (soryuusi0219 の 419359 の前半)。
// 各レベル i に代表元 T[i][x] (i を x に写す元)、その逆、軌道 orbit[i]、生成元 S[i]、
// 処理済みの (軌道点, 生成元) の数 done[i][x] を持つ。生成元を足すたびに、まだ試して
// いない組だけを process に流す。位数は各レベルの軌道の大きさの積。
struct Solver {
  static constexpr unsigned MOD = 998244353;
  using Perm = std::array<int, 50>;

  int n;
  vector<vector<int>> gen;
  unsigned ans = 1;

  vector<vector<Perm>> T, invT;
  vector<vector<char>> has;
  vector<vector<int>> orbit;
  vector<vector<Perm>> S;
  vector<vector<int>> done;

  Solver(int n_, const vector<vector<int>> &g) : n(n_), gen(g) {}

  Perm identity() const {
    Perm p{};
    for (int i = 0; i < n; ++i) p[i] = i;
    return p;
  }
  Perm compose(const Perm &a, const Perm &b) const {
    Perm c{};
    for (int i = 0; i < n; ++i) c[i] = a[b[i]];
    return c;
  }
  Perm inverse(const Perm &p) const {
    Perm q{};
    for (int i = 0; i < n; ++i) q[p[i]] = i;
    return q;
  }

  // レベル start 以降で sift する。
  bool contains(Perm g, int start) const {
    for (int i = start; i < n; ++i) {
      int x = g[i];
      if (x == i) continue;
      if (!has[i][x]) return false;
      g = compose(invT[i][x], g);
    }
    return true;
  }

  // g をレベル i の代表元系で表す (Algorithm B)。
  void process(int i, Perm g) {
    if (i == n) return;
    int x = g[i];
    if (!has[i][x]) {
      T[i][x] = g;
      invT[i][x] = inverse(g);
      has[i][x] = 1;
      orbit[i].push_back(x);
      return;
    }
    Perm h = (x == i) ? g : compose(invT[i][x], g);
    if (!contains(h, i + 1)) extend(i + 1, h);
  }

  // レベル i の群に生成元 g を足す (Algorithm A)。
  void extend(int i, Perm g) {
    S[i].push_back(g);
    for (size_t k = 0; k < orbit[i].size(); ++k) {
      int x = orbit[i][k];
      while (done[i][x] < (int)S[i].size()) {
        int j = done[i][x]++;
        process(i, compose(S[i][j], T[i][x]));
      }
    }
  }

  // 入力の生成元だけで S_n を証明する。失敗したら false を返し、通常の鎖に落とす。
  bool certifies_symmetric() const {
    if (n < 3 || gen.size() < 2) return false;
    int a = -1, b = -1;
    for (const vector<int> &g : gen) {
      vector<char> seen(n, 0);
      int two = 0, A = -1, B = -1;
      bool ok = true;
      for (int i = 0; i < n && ok; ++i) {
        if (seen[i]) continue;
        int len = 0, j = i;
        do {
          seen[j] = 1;
          j = g[j];
          ++len;
        } while (j != i);
        if (len == 2) {
          ++two, A = i, B = g[i];
        } else if (len % 2 == 0) {
          ok = false;
        }
      }
      if (ok && two == 1) {
        a = A, b = B;
        break;
      }
    }
    if (a < 0) return false;
    // 転置 (a b) の共役でできる辺を、統合する分だけたどる。
    vector<int> par(n, -1);
    auto find = [&](int x) {
      int r = x;
      while (par[r] >= 0) r = par[r];
      while (x != r) {
        int p = par[x];
        par[x] = r;
        x = p;
      }
      return r;
    };
    auto unite = [&](int x, int y) {
      x = find(x), y = find(y);
      if (x == y) return false;
      if (par[x] > par[y]) std::swap(x, y);
      par[x] += par[y], par[y] = x;
      return true;
    };
    unite(a, b);
    vector<pair<int, int>> edges{{a, b}};
    for (size_t ei = 0; ei < edges.size(); ++ei) {
      auto [u, v] = edges[ei];
      for (const vector<int> &p : gen) {
        int x = p[u], y = p[v];
        if (!unite(x, y)) continue;
        edges.emplace_back(x, y);
        if ((int)edges.size() == n - 1) return true;
      }
    }
    return false;
  }

  void run() {
    if (certifies_symmetric()) {
      unsigned long long f = 1;
      for (int i = 2; i <= n; ++i) f = f * (unsigned)i % MOD;
      ans = (unsigned)f;
      return;
    }
    T.assign(n, vector<Perm>(n));
    invT.assign(n, vector<Perm>(n));
    has.assign(n, vector<char>(n, 0));
    orbit.assign(n, {});
    S.assign(n, {});
    done.assign(n, vector<int>(n, 0));
    const Perm e = identity();
    for (int i = 0; i < n; ++i) {
      T[i][i] = e;
      invT[i][i] = e;
      has[i][i] = 1;
      orbit[i].push_back(i);
    }
    for (const vector<int> &g : gen) {
      Perm p{};
      for (int i = 0; i < n; ++i) p[i] = g[i];
      if (!contains(p, 0)) extend(0, p);
    }
    ans = 1;
    for (int i = 0; i < n; ++i)
      ans = (unsigned)((unsigned long long)ans * (unsigned)orbit[i].size() % MOD);
  }

  unsigned answer() const { return ans; }
};
