#pragma once
// 今の Library には辺を足しながら増加路を 1 本ずつ探す口が無いので、二部マッチング (mylib/graph/BipartiteGraph.hpp の
// bipartite_matching) を、前のマッチングを渡して解き直す。日を逆にたどり、mex を 1 つ伸ばせるかを試すたびに、力 0 から
// mex までを左、部を右にしたグラフを、残っている学生の辺で組み直す。伸ばせなかったときは前のマッチングを残す (大きさが
// 同じ別のマッチングは、力 0 から mex - 1 を全部覆うとは限らない)。
#include "mylib/graph/BipartiteGraph.hpp"

struct Solver {
  int n, m;
  const vector<int> &p, &c, &k;
  Solver(int n, int m, const vector<int> &p, const vector<int> &c, const vector<int> &k) : n(n), m(m), p(p), c(c), k(k) {}
  vector<int> run() {
    const int d = k.size();
    vector<char> leave(n, 0);
    for (int x : k) leave[x] = 1;
    vector<int> here;  // 残っている学生
    for (int i = 0; i < n; ++i)
      if (!leave[i]) here.push_back(i);
    vector<int> ml;  // 力 v の相手の部
    auto extend = [&](int mex) {
      const int L = mex + 1;
      BipartiteGraph bg(L, m);
      for (int i : here)
        if (p[i] <= mex) bg.add_edge(p[i], L + c[i]);
      vector<int> partner(L + m, -1);
      for (int v = 0; v < mex; ++v) partner[v] = L + ml[v], partner[L + ml[v]] = v;
      auto [used, pt] = bipartite_matching(bg, partner);
      if ((int)used.size() < L) return false;
      ml.resize(L);
      for (int v = 0; v < L; ++v) ml[v] = pt[v] - L;
      return true;
    };
    vector<int> ans(d);
    int mex = 0;
    for (int day = d - 1; day >= 0; --day) {
      while (mex < 5000 && extend(mex)) ++mex;
      ans[day] = mex;
      here.push_back(k[day]);
    }
    return ans;
  }
};
