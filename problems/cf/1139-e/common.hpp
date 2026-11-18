#pragma once
// cf-1139-e の Kuhn 法の提出が共有するもの。力の値 (5000 個) を左、部を右に置き、学生を辺 (力, 部) にする。日を逆に
// たどり、最後の日に残っている学生の辺で始めて、mex の値から増加路が見つかるあいだ mex を増やして答えを書き、その日に
// 去った学生の辺を足す。辺は増えるだけなので mex は減らない。Match は _shared/kuhn/inc_matching.hpp の IncMatching の
// どれかで、提出が決めてから include する。
#include <array>
#include <vector>

struct Solver {
  int n, m;
  const vector<int> &p, &c, &k;
  Solver(int n, int m, const vector<int> &p, const vector<int> &c, const vector<int> &k) : n(n), m(m), p(p), c(c), k(k) {}
  vector<int> run() {
    constexpr int V = 5000;
    const int d = k.size();
    vector<char> leave(n, 0);
    for (int x : k) leave[x] = 1;
    vector<array<int, 2>> e;
    for (int i = 0; i < n; ++i)
      if (!leave[i]) e.push_back({p[i], c[i]});
    Match mt(V, m, e);
    vector<int> ans(d);
    int mex = 0;
    for (int day = d - 1; day >= 0; --day) {
      while (mex < V && mt.augment(mex)) ++mex;
      ans[day] = mex;
      mt.add_edge(p[k[day]], c[k[day]]);
    }
    return ans;
  }
};
