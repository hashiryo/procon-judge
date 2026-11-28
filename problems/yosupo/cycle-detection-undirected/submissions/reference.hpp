#pragma once
// 無向グラフの閉路を 1 つ見つける。深さ優先で、いま道の上にある頂点へ戻る辺を見つけたら、
// その頂点から今の頂点までの木の辺を逆向きに並べ、最後にその辺を足す。
// 多重辺があるので、親を頂点ではなく「入ってきた辺の番号」で除く。
#include <vector>

struct Solver {
  int n;
  std::vector<int> head, nxt, to, eu, ev;
  std::vector<int> vs, es;

  Solver(int n_, const std::vector<int> &u, const std::vector<int> &v)
      : n(n_), eu(u), ev(v) {
    const int m = (int)u.size();
    head.assign(n, -1);
    nxt.resize(2 * m);
    to.resize(2 * m);
    for (int i = 0; i < m; ++i) {
      to[2 * i] = v[i], nxt[2 * i] = head[u[i]], head[u[i]] = 2 * i;
      to[2 * i + 1] = u[i], nxt[2 * i + 1] = head[v[i]], head[v[i]] = 2 * i + 1;
    }
  }

  void run() {
    vs.clear(), es.clear();
    const int m = (int)eu.size();
    std::vector<signed char> col(n, 0);
    std::vector<int> par(n, -1), pe(n, -1), it(n, 0), st;
    st.reserve(n);
    for (int s = 0; s < n; ++s) {
      if (col[s]) continue;
      col[s] = 1;
      st.push_back(s);
      it[s] = head[s];
      while (!st.empty()) {
        const int x = st.back();
        int &e = it[x];
        if (e == -1) {
          col[x] = 2;
          st.pop_back();
          continue;
        }
        const int ei = e / 2;
        const int dir = e & 1;
        e = nxt[e];
        if (ei == pe[x]) continue;  // 入ってきた辺は除く (多重辺は残す)
        const int y = to[ei * 2 + dir];
        if (col[y] == 0) {
          col[y] = 1;
          par[y] = x;
          pe[y] = ei;
          it[y] = head[y];
          st.push_back(y);
        } else if (col[y] == 1) {
          // y から x までの木の辺を逆向きに並べ、最後に ei を足す。
          std::vector<int> vv, ee;
          for (int t = x; t != y; t = par[t]) vv.push_back(t), ee.push_back(pe[t]);
          vv.push_back(y);
          ee.push_back(ei);  // ee[i] が vv[i] と vv[i+1] を結ぶ。最後が vv.back() と vv[0]。
          vs = std::move(vv), es = std::move(ee);
          return;
        }
      }
    }
  }

  const std::vector<int> &vertex() const { return vs; }
  const std::vector<int> &edge() const { return es; }
};
