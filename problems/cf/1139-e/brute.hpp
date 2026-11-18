#pragma once
// 愚直解 (pj testdata crosscheck 用)。学生が 10 人ほどまでの入力で、日ごとに、力 0 から j - 1 までに別々の部の残っている
// 学生を割り当てられるかを全部試し、割り当てられる j の最大を出す。
#include <cstdlib>
#include <vector>

struct Solver {
  int n, m;
  const vector<int> &p, &c, &k;
  Solver(int n, int m, const vector<int> &p, const vector<int> &c, const vector<int> &k) : n(n), m(m), p(p), c(c), k(k) {}
  vector<int> run() {
    if (n > 12) abort();
    vector<char> gone(n, 0), club(m, 0);
    auto rec = [&](auto &&self, int v, int j) -> bool {
      if (v == j) return true;
      for (int i = 0; i < n; ++i)
        if (!gone[i] && p[i] == v && !club[c[i]]) {
          club[c[i]] = 1;
          const bool ok = self(self, v + 1, j);
          club[c[i]] = 0;
          if (ok) return true;
        }
      return false;
    };
    vector<int> ans;
    for (int x : k) {
      gone[x] = 1;
      int j = 0;
      while (rec(rec, 0, j + 1)) ++j;
      ans.push_back(j);
    }
    return ans;
  }
};
