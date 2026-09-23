#pragma once
// 提出が共有するもの。解き方は同じで Union-Find の実装だけが違うので、解法を
// ここにテンプレートで 1 つ置いて、提出は Union-Find の型を選ぶだけにする。
#include <utility>
#include "pj.hpp"
#include "mylib/algebra/Nimber.hpp"

constexpr i64 MOD = 998244353;
constexpr int BITS = 30;

// 重みの型は Library のテストと同じく Nimber。足し算が xor で逆元が自分自身
// なので、ポテンシャル付き Union-Find にそのまま載る。
//
// 成分ごとに「根からの xor の各ビットが立っている頂点の数」を持つ。全対の距離
// の和はビットごとに (立っている数) x (立っていない数) x 2^i。成分を繋ぐとき、
// 吸収される側の頂点は根の間の xor のぶんだけビットが反転する。
template <class UF> struct PathQuerySolver {
  int n;
  UF uf;
  vector<array<int, BITS>> cnt;

  explicit PathQuerySolver(int n) : n(n), uf(n), cnt(n) { Nimber::init(); }

  void link(int x, int v, i64 w) {
    int a = uf.leader(x), b = uf.leader(v);
    int sa = uf.size(a), sb = uf.size(b);
    Nimber d = uf.diff(v, x) + Nimber((u64)w);  // 根 a と根 b の間の xor
    uf.unite(a, b, d);
    int root = uf.leader(a);
    int other = root == a ? b : a;
    int other_size = root == a ? sb : sa;
    u64 dv = d.val();
    for (int i = 0; i < BITS; ++i)
      cnt[root][i] += (dv >> i) & 1 ? other_size - cnt[other][i] : cnt[other][i];
  }

  i64 dist(int u, int v) {
    return uf.connected(u, v) ? (i64)uf.diff(u, v).val() : -1;
  }

  i64 pair_sum(int v) {
    int r = uf.leader(v), sz = uf.size(r);
    i64 ans = 0;
    for (int i = 0; i < BITS; ++i)
      ans = (ans + (i64)cnt[r][i] * (sz - cnt[r][i]) % MOD * ((i64)1 << i)) % MOD;
    return ans;
  }
};
