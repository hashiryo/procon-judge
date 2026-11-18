#pragma once
// 今の Library には頂点を消しながら増加路を探す口が無いので、二部マッチング (mylib/graph/BipartiteGraph.hpp の
// bipartite_matching) を、前のマッチングを渡して解き直す。頂点の選び方と遊びの言い換えは submissions/reference.hpp と
// 同じ。空きマスの頂点 v を消すたびに、残りの頂点でグラフを組み直し、v の組を外したマッチングを渡して最大マッチングを
// 求める。大きさが減れば、v はどの最大マッチングにも入る。
#include <string>
#include <vector>
#include "mylib/graph/BipartiteGraph.hpp"

struct Solver {
  int n, m;
  const vector<string> &b;
  const vector<array<int, 2>> &mv;
  Solver(int n, int m, const vector<string> &board, const vector<array<int, 2>> &moves) : n(n), m(m), b(board), mv(moves) {}
  vector<int> run() {
    const int N = n * m;
    int sx = 0, sy = 0;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j)
        if (b[i][j] == '.') sx = i, sy = j;
    vector<int> id(N, -1);
    vector<char> side(N, 0), del(N, 0);
    int nl = 0, nr = 0;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j) {
        const int v = i * m + j, d = (i + j + sx + sy) & 1;
        if (d ? b[i][j] == 'O' : b[i][j] != 'O') side[v] = d, id[v] = d ? nr++ : nl++;
      }
    auto gid = [&](int v) { return side[v] ? nl + id[v] : id[v]; };  // BipartiteGraph の番号
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    vector<int> partner(nl + nr, -1);
    auto solve = [&]() {
      BipartiteGraph bg(nl, nr);
      for (int v = 0; v < N; ++v)
        if (id[v] >= 0 && !side[v] && !del[v])
          for (int d = 0; d < 4; ++d) {
            const int x = v / m + dx[d], y = v % m + dy[d];
            if (x < 0 || x >= n || y < 0 || y >= m) continue;
            const int w = x * m + y;
            if (id[w] >= 0 && !del[w]) bg.add_edge(id[v], nl + id[w]);
          }
      auto [used, p] = bipartite_matching(bg, partner);
      partner = p;
      return (int)used.size();
    };
    int cur = solve();
    const int T = mv.size();
    vector<char> win(T);
    for (int t = 0; t < T; ++t) {
      const int v = t == 0 ? sx * m + sy : mv[t - 1][0] * m + mv[t - 1][1];
      del[v] = 1;
      if (const int g = gid(v), u = partner[g]; u >= 0) partner[g] = partner[u] = -1;
      const int nx = solve();
      win[t] = nx < cur, cur = nx;
    }
    vector<int> res;
    for (int i = 0; i + 1 < T; i += 2)
      if (win[i] && win[i + 1]) res.push_back(i / 2 + 1);
    return res;
  }
};
