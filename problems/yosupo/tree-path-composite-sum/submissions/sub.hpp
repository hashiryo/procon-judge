#pragma once
#include "_reroot.hpp"
#include "neo/base/core/algebra/ZMod.hpp"

// 比べるための特化した形。Library Checker の首位 (nandhagk の 346214) と同じく、和が引き算できることを使う。葉から剥がした順
// (_reroot.hpp の peel) で下から部分木の和 dp と大きさを集め、逆順で親の側の値を、親の全体から自分の寄与を引いて作る。
// 兄弟を除いた積を作らないので、全方位木 DP の汎用の形が届く下限の目安になる。
struct Solver {
  using Z = ZMod<998244353>;
  int n;
  vector<Z> a, b, c;
  vector<array<int, 2>> edges;
  vector<u32> ans;
  Solver(int n, const vector<u32> &a_, const vector<array<int, 2>> &e, const vector<u32> &b_, const vector<u32> &c_)
      : n(n), a(a_.begin(), a_.end()), b(b_.begin(), b_.end()), c(c_.begin(), c_.end()), edges(e), ans(n) {}
  void run() {
    vector<int> ord(n), par(n, -1), pe(n, -1), siz(n, 1);
    reroot_exp::peel(n, edges, ord.data(), par.data(), pe.data());
    vector<Z> dp(a), tp(n), kp(n);
    for (int i = 0; i + 1 < n; ++i) {
      const int v = ord[i], p = par[v], e = pe[v];
      tp[v] = b[e] * dp[v] + c[e] * Z(siz[v]), dp[p] += tp[v], siz[p] += siz[v];
    }
    for (int i = n - 1; i--;) {
      const int v = ord[i], p = par[v], e = pe[v];
      kp[v] = b[e] * (kp[p] + dp[p] - tp[v]) + c[e] * Z(n - siz[v]);
    }
    for (int v = 0; v < n; ++v) ans[v] = (kp[v] + dp[v]).val();
  }
  const vector<u32> &answer() const { return ans; }
};
