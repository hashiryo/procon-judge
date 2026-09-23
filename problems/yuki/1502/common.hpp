#pragma once
// 提出が共有するもの。解き方は同じで Union-Find の実装だけが違うので、解法を
// ここにテンプレートで 1 つ置いて、提出は Union-Find の型を選ぶだけにする。
#include <algorithm>
#include <utility>
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/algebra/Algebra.hpp"

using Mint = ModInt<1000000007>;

// x -> (neg ? -x : x) + shift の合成。add(a, b) は a を先に、b を後に適用する。
// 符号を反転するものがあるので可換ではない。
struct SignedShift {
  using T = pair<bool, i64>;
  static constexpr T o = {false, 0};
  static T add(const T &a, const T &b) {
    if (b.first) return {!a.first, b.second - a.second};
    return {a.first, a.second + b.second};
  }
  static T neg(const T &a) { return {a.first, a.first ? a.second : -a.second}; }
};
using G = Algebra<SignedShift>;

// 代表 t に対する各要素の potential (a, b) は A_i = (a ? -t : t) + b を表す。
template <class UF> struct CountSolver {
  int n;
  i64 k;
  vector<array<i64, 3>> eqs;
  UF uf;
  Mint ans = 0;

  CountSolver(int n, i64 k, const vector<array<i64, 3>> &eqs)
      : n(n), k(k), eqs(eqs), uf(n) {}

  void run() {
    for (auto &[x, y, z] : eqs) {
      if (uf.connected((int)x, (int)y)) continue;
      uf.unite((int)x, (int)y, G(make_pair(true, z)));
    }
    ans = count_upto(k) - count_upto(k - 1);
  }

  i64 answer() const { return (i64)ans.val(); }

  // 全部の値が [1, upper] に収まる列の個数。
  Mint count_upto(i64 upper) {
    vector<i64> lb(n, -((i64)1 << 60)), ub(n, (i64)1 << 60);
    for (int i = 0; i < n; ++i) {
      int v = uf.leader(i);
      auto [a, b] = uf.potential(i).x;
      if (a) lb[v] = max(lb[v], b - upper), ub[v] = min(ub[v], b - 1);
      else lb[v] = max(lb[v], 1 - b), ub[v] = min(ub[v], upper - b);
    }
    // 閉路を作る式は t を決めるか、t に関係なく成否が決まる。
    for (auto &[x, y, z] : eqs) {
      auto [xa, xb] = uf.potential((int)x).x;
      auto [ya, yb] = uf.potential((int)y).x;
      i64 q = z - xb - yb;  // (±t) + (±t) = q
      if (xa != ya) {       // 符号が逆なら t が消える
        if (q) return 0;
        continue;
      }
      if (q & 1) return 0;
      if (xa) q = -q;
      q /= 2;
      if (q < 0 || upper < q) return 0;
      int v = uf.leader((int)x);
      lb[v] = max(lb[v], q), ub[v] = min(ub[v], q);
    }
    Mint ret = 1;
    for (int i = 0; i < n; ++i)
      if (uf.leader(i) == i) ret *= max((i64)0, ub[i] - lb[i] + 1);
    return ret;
  }
};
