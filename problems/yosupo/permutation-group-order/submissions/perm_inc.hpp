#pragma once
#include <array>

#include "pj.hpp"

// incremental Schreier–Sims (Knuth の Algorithm A/B)。TKO919 の 420147 と同じ形。
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

  void run() {
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
