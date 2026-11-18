#pragma once
// 参照実装。辺の集合を k 本の森に分ける (グラフ的マトロイドの和) のを、Edmonds のマトロイドの分割の算法で求める。
// 提出の共通部分 (problems/_shared/matroid/) を読まずに書いた。
//
// 辺を番号の順に 1 本ずつ入れる。どの森にもそのまま入らなければ、交換の列を幅優先で探す。辺 u を森 j に入れると、森 j
// の中の u の両端を結ぶ道の辺 y が押し出され、y をほかの森に入れ直す。どこかの森にそのまま入る辺に着いたら、列に沿って
// 入れ替える (幅優先の最短の列なので、どの森も森のまま)。見つからない辺は使わない。
// 見つからなかった探索でたどった辺の集合 S は、S のどの辺を S の外の森に入れても S の辺しか押し出さないので、以後の
// どの探索の最短の列にも現れない。そこで以後の探索では飛ばす。
#include <algorithm>
#include <array>
#include <vector>

struct Solver {
  int n, k;
  const vector<array<int, 2>> &e;
  vector<int> own;

  Solver(int n, const vector<array<int, 2>> &edges, int k) : n(n), k(k), e(edges) {}

  void run() {
    const int m = e.size();
    own.assign(m, -1);
    // 森ごとの辺の一覧 (pos は一覧の中の位置) と、根付き木にした形 (親の頂点、親への辺、深さ、根)
    vector<vector<int>> mem(k), up(k, vector<int>(n)), upe(k, vector<int>(n)), dep(k, vector<int>(n)), root(k, vector<int>(n));
    vector<int> pos(m, -1), head(n), nxt, to, ev, q, from(m), seen(m, 0);
    vector<char> stale(k, 1), dead(m, 0);
    int stamp = 0;
    auto put = [&](int x, int j) {  // 辺 x を森 own[x] から森 j へ移す (-1 は使わない)
      if (own[x] >= 0) {
        auto &v = mem[own[x]];
        const int last = v.back();
        v[pos[x]] = last, pos[last] = pos[x], v.pop_back();
        stale[own[x]] = 1;
      }
      own[x] = j;
      if (j >= 0) pos[x] = mem[j].size(), mem[j].push_back(x), stale[j] = 1;
    };
    auto rebuild = [&](int j) {
      if (!stale[j]) return;
      stale[j] = 0;
      fill(head.begin(), head.end(), -1);
      nxt.clear(), to.clear(), ev.clear();
      for (int x : mem[j])
        for (int s = 0; s < 2; ++s) {
          const int a = e[x][s], b = e[x][s ^ 1];
          nxt.push_back(head[a]), to.push_back(b), ev.push_back(x), head[a] = nxt.size() - 1;
        }
      auto &U = up[j], &UE = upe[j], &D = dep[j], &R = root[j];
      fill(R.begin(), R.end(), -1);
      vector<int> st;
      for (int r = 0; r < n; ++r) {
        if (R[r] >= 0) continue;
        R[r] = r, U[r] = -1, UE[r] = -1, D[r] = 0;
        st.assign(1, r);
        while (!st.empty()) {
          const int v = st.back();
          st.pop_back();
          for (int t = head[v]; t >= 0; t = nxt[t]) {
            const int w = to[t];
            if (R[w] >= 0) continue;
            R[w] = r, U[w] = v, UE[w] = ev[t], D[w] = D[v] + 1;
            st.push_back(w);
          }
        }
      }
    };
    for (int s = 0; s < m; ++s) {
      for (int j = 0; j < k; ++j) rebuild(j);
      ++stamp;
      seen[s] = stamp, from[s] = -1;
      q.assign(1, s);
      int got = -1, got_j = -1;
      for (size_t h = 0; h < q.size() && got < 0; ++h) {
        const int u = q[h];
        for (int j = 0; j < k && got < 0; ++j) {
          if (j == own[u]) continue;
          int a = e[u][0], b = e[u][1];
          if (root[j][a] != root[j][b]) {
            got = u, got_j = j;
            break;
          }
          while (a != b) {
            if (dep[j][a] < dep[j][b]) swap(a, b);
            const int y = upe[j][a];
            if (seen[y] != stamp && !dead[y]) seen[y] = stamp, from[y] = u, q.push_back(y);
            a = up[j][a];
          }
        }
      }
      if (got < 0) {
        for (int y : q) dead[y] = 1;
        continue;
      }
      for (int x = got, j = got_j; x >= 0;) {
        const int old = own[x], p = from[x];
        put(x, j);
        j = old, x = p;
      }
    }
  }

  vector<int> answer() const { return own; }
};
