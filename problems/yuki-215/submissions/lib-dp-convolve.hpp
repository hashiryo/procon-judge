#pragma once
#include "pj.hpp"
#include "mylib/algebra/ModInt.hpp"
#include "mylib/fft/bostan_mori.hpp"
#include "mylib/fft/convolve.hpp"

// 配列の DP で素数側と合成数側の分布を別々に作り、1 回の畳み込みで合わせる。
// 分子は累積和を手で取る。多項式クラスを介さないので中間の多項式が出ない。
struct Solver {
  i64 n;
  int p, c;
  i64 ans = 0;

  Solver(i64 n, int p, int c) : n(n), p(p), c(c) {}

  void run() {
    using Mint = ModInt<1000000007>;
    const int max_p = 13 * p + 1, max_c = 12 * c + 1;
    vector<vector<Mint>> dp1(p + 1, vector<Mint>(max_p)), dp2(c + 1, vector<Mint>(max_c));
    dp1[0][0] = 1;
    dp2[0][0] = 1;
    for (int v : {2, 3, 5, 7, 11, 13})
      for (int i = 0; i < p; i++)
        for (int j = 0; j + v < max_p; j++) dp1[i + 1][j + v] += dp1[i][j];
    for (int v : {4, 6, 8, 9, 10, 12})
      for (int i = 0; i < c; i++)
        for (int j = 0; j + v < max_c; j++) dp2[i + 1][j + v] += dp2[i][j];
    auto f = convolve<Mint, 1 << 17>(dp1[p], dp2[c]), g = f;
    const int d = (int)f.size();
    for (int i = 0; i < d; i++) f[0] -= f[i];
    for (int i = 1; i < d; i++) f[i] += f[i - 1];
    g[0] -= 1;
    ans = div_at<Mint, 1 << 17>(f, g, n - 1).val();
  }

  i64 answer() const { return ans; }
};
