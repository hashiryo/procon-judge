#pragma once
#include "pj.hpp"

// 置換は p[i] を i の行き先とする。合成は (p q)[i] = p[q[i]]。
//
// 強生成集合を積の閉包で作る Schreier–Sims (soryuusi0219 の 419355 と同じ形)。
// レベル k は「k+1..n-1 を固定し、k を動かす」元を 1 つずつ持つ。table[k][j] は
// その元の逆置換 (0..k の像) で、j = k の行き先 (< k)。すべての 2 つの積を足し込み、
// 新しい元が入らなくなるまで閉じる。位数は各レベルの元の数 (軌道の大きさ) の積。
struct Solver {
  static constexpr unsigned MOD = 998244353;
  int n;
  vector<vector<int>> gen;
  unsigned ans = 1;

  Solver(int n_, const vector<vector<int>> &g) : n(n_), gen(g) {}

  void run() {
    vector<vector<vector<int>>> table(n);
    for (int k = 0; k < n; ++k) table[k].assign(k, {});
    vector<int> idx_sz(n, 1);         // 各レベルの元の数。恒等の分だけ 1 から始める
    vector<pair<int, int>> entries;   // 見つけた元の (k, j) を入れた順
    vector<int> p;

    // 既にある元で上から順に打ち消す。足りなければそのレベルを返す。
    auto sift = [&](vector<int> &q) -> int {
      for (int k = (int)q.size() - 1; k >= 0; --k) {
        int j = q[k];
        if (j == k) continue;
        const auto &t = table[k][j];
        if (t.empty()) return k;
        for (int i = 0; i < k; ++i) q[i] = t[q[i]];
      }
      return -1;
    };
    auto insert = [&](vector<int> q) {
      int k = sift(q);
      if (k < 0) return;              // 既に張られる
      int j = q[k];
      table[k][j].resize(k + 1);
      for (int i = 0; i <= k; ++i) table[k][j][q[i]] = i;  // 逆置換で持つ
      entries.emplace_back(k, j);
      ++idx_sz[k];
    };

    for (auto &g : gen) insert(g);

    // level(a) <= level(b) の a∘b を入れる。table は逆置換なので、その積は
    // (a∘b)^{-1} = b^{-1}∘a^{-1} を書いてから逆に読む。
    auto insert_product = [&](int k0, int j0, int k1, int j1) {
      const auto &t0 = table[k0][j0];
      const auto &t1 = table[k1][j1];
      p.assign(k1 + 1, 0);
      for (int i = 0; i <= k0; ++i) p[t1[t0[i]]] = i;
      for (int i = k0 + 1; i <= k1; ++i) p[t1[i]] = i;
      insert(p);
    };
    for (size_t done = 0; done < entries.size(); ++done) {
      auto [k0, j0] = entries[done];
      for (size_t y = 0; y <= done; ++y) {
        auto [k1, j1] = entries[y];
        if (k0 <= k1) insert_product(k0, j0, k1, j1);
        if (y != done && k1 <= k0) insert_product(k1, j1, k0, j0);
      }
    }

    for (int k = 0; k < n; ++k)
      ans = (unsigned)((unsigned long long)ans * (unsigned)idx_sz[k] % MOD);
  }

  unsigned answer() const { return ans; }
};
