#pragma once
// Kuhn 法の提出が共有するもの。頂点の選び方と遊びの言い換えは submissions/reference.hpp の説明と同じ。偶奇の差が 0 の
// 頂点を左、1 の頂点を右に置き、最初に augment() を繰り返して最大マッチングを作る。空きマスの頂点 v を消すときは、v に
// 付いた辺を erase_edge で全部抜いてから augment() を 1 回呼ぶ。v に相手がいて、増加路が見つからなければ、v はどの最大
// マッチングにも入る (見つかる増加路は、組を外された相手を端に持つ)。Match は _shared/kuhn/inc_matching.hpp の
// IncMatching のどれかで、提出が決めてから include する。
#include <string>
#include <vector>

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
    vector<char> side(N, 0);
    int nl = 0, nr = 0;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j) {
        const int v = i * m + j, d = (i + j + sx + sy) & 1;
        if (d ? b[i][j] == 'O' : b[i][j] != 'O') side[v] = d, id[v] = d ? nr++ : nl++;
      }
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    auto each = [&](int v, auto &&f) {
      for (int d = 0; d < 4; ++d) {
        const int x = v / m + dx[d], y = v % m + dy[d];
        if (x >= 0 && x < n && y >= 0 && y < m && id[x * m + y] >= 0) f(x * m + y);
      }
    };
    vector<array<int, 2>> e;
    for (int v = 0; v < N; ++v)
      if (id[v] >= 0 && !side[v]) each(v, [&](int w) { e.push_back({id[v], id[w]}); });
    Match mt(nl, nr, e);
    while (mt.augment()) {}
    const int T = mv.size();
    vector<char> win(T);
    for (int t = 0; t < T; ++t) {
      const int v = t == 0 ? sx * m + sy : mv[t - 1][0] * m + mv[t - 1][1];
      const bool matched = side[v] ? mt.mate_right(id[v]) >= 0 : mt.mate_left(id[v]) >= 0;
      each(v, [&](int w) { side[v] ? mt.erase_edge(id[w], id[v]) : mt.erase_edge(id[v], id[w]); });
      win[t] = matched && !mt.augment();
    }
    vector<int> res;
    for (int i = 0; i + 1 < T; i += 2)
      if (win[i] && win[i + 1]) res.push_back(i / 2 + 1);
    return res;
  }
};
