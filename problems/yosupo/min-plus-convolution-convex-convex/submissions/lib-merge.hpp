#pragma once
#include "pj.hpp"

// 凸な列どうしの min-plus convolution は、差分の列を併合すると O(N+M) で作れる。
// a[i+1]-a[i] と b[j+1]-b[j] はどちらも広義単調増加なので、2 つを併合した列の前からの
// 累積和が c の隣り合う差になる。c[0] = a[0] + b[0]。
struct Solver {
  vector<i64> a, b, c;

  Solver(const vector<i64> &a, const vector<i64> &b) : a(a), b(b) {}

  void run() {
    const int n = (int)a.size(), m = (int)b.size();
    c.resize(n + m - 1);
    c[0] = a[0] + b[0];
    int i = 0, j = 0, k = 1;
    while (i + 1 < n && j + 1 < m) {
      if (a[i + 1] - a[i] <= b[j + 1] - b[j]) c[k] = c[k - 1] + (a[i + 1] - a[i]), ++i;
      else c[k] = c[k - 1] + (b[j + 1] - b[j]), ++j;
      ++k;
    }
    while (i + 1 < n) c[k] = c[k - 1] + (a[i + 1] - a[i]), ++i, ++k;
    while (j + 1 < m) c[k] = c[k - 1] + (b[j + 1] - b[j]), ++j, ++k;
  }

  const vector<i64> &answer() const { return c; }
};
