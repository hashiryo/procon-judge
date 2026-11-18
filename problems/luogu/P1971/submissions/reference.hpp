#pragma once
// 期待出力を作る参照実装。_shared/kuhn の核とも今の Library とも別に書いた、よく知られた解き方 (2026-11-18)。
//
// 空きマスの始めの位置との偶奇の差が 0 のマスは、空きマスが来るときいつも黒の駒を動かされ (始めの位置を含む)、差が 1 の
// マスは白の駒を動かされる。そこで、差が 0 の黒のマスと始めの位置、差が 1 の白のマスだけを頂点にし、隣り合う頂点を辺で
// 結ぶ。空きマスが一度通ったマスには、偶奇の合わない色の駒が入るので二度と通らない。この遊びは、残りのグラフで駒 (空き
// マス) を隣の頂点へ動かし、動かせない人が負けるもの (頂点を消していく undirected vertex geography) で、手番の人が
// 勝てるのは、今の頂点が残りのグラフのどの最大マッチングにも入るとき。最大マッチングを保ち、空きマスの頂点 v を消す
// たびに、v に相手 u がいれば組を外して u から増加路を探す (再帰の深さ優先、左右どちらの側からでも探せる)。見つからな
// ければ v はどの最大マッチングにも入る。小さい入力では brute.hpp (盤の上で手を全部試す) と、gen.py の seed 1000 から
// 2999 で突き合わせてある。
#include <string>
#include <vector>

struct Solver {
  int n, m;
  const vector<string> &b;
  const vector<array<int, 2>> &mv;
  vector<char> ok, del;
  vector<int> mate, vis;
  int stamp = 0;
  Solver(int n, int m, const vector<string> &board, const vector<array<int, 2>> &moves) : n(n), m(m), b(board), mv(moves) {}
  bool dfs(int v) {
    const int x = v / m, y = v % m;
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    for (int d = 0; d < 4; ++d) {
      const int nx = x + dx[d], ny = y + dy[d];
      if (nx < 0 || nx >= n || ny < 0 || ny >= m) continue;
      const int w = nx * m + ny;
      if (!ok[w] || del[w] || vis[w] == stamp) continue;
      vis[w] = stamp;
      if (mate[w] < 0 || dfs(mate[w])) return mate[w] = v, mate[v] = w, true;
    }
    return false;
  }
  vector<int> run() {
    const int N = n * m;
    int sx = 0, sy = 0;
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j)
        if (b[i][j] == '.') sx = i, sy = j;
    ok.assign(N, 0), del.assign(N, 0), mate.assign(N, -1), vis.assign(N, 0);
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j) {
        const int d = (i + j + sx + sy) & 1;
        ok[i * m + j] = d ? b[i][j] == 'O' : b[i][j] != 'O';
      }
    for (int v = 0; v < N; ++v)
      if (ok[v] && !(((v / m) + (v % m) + sx + sy) & 1) && mate[v] < 0) ++stamp, dfs(v);
    const int T = mv.size();
    vector<char> win(T);
    for (int t = 0; t < T; ++t) {
      const int v = t == 0 ? sx * m + sy : mv[t - 1][0] * m + mv[t - 1][1];
      del[v] = 1;
      const int u = mate[v];
      if (u < 0) continue;
      mate[v] = mate[u] = -1, ++stamp;
      win[t] = !dfs(u);
    }
    vector<int> res;
    for (int i = 0; i + 1 < T; i += 2)
      if (win[i] && win[i + 1]) res.push_back(i / 2 + 1);
    return res;
  }
};
