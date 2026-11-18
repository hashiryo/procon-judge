#pragma once
// 愚直解 (pj testdata crosscheck 用)。漁師が 8 人ほどまでの入力で、言う順番を全部試し、問題文のとおりに言う値 (前の値より
// 大きい a の倍数の最小) を求めて、和の最小を出す。
#include <algorithm>
#include <cstdlib>
#include <numeric>
#include <vector>

struct Solver {
  int n;
  const vector<long long> &a;
  Solver(int n, const vector<long long> &a) : n(n), a(a) {}
  long long run() {
    if (n > 9) abort();
    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    long long best = -1;
    do {
      long long b = 0, sum = 0;
      for (int i : p) b = (b / a[i] + 1) * a[i], sum += b;
      if (best < 0 || sum < best) best = sum;
    } while (next_permutation(p.begin(), p.end()));
    return best;
  }
};
