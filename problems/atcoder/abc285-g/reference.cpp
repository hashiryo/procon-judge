// 期待出力を作る参照実装。マスを (i + j) の偶奇で黒と白に分け、1 でない隣り合うマスの組を辺にした二部グラフで、2 のマスを
// 全部覆うマッチングがあるかを見る。Mendelsohn–Dulmage の定理から、黒の 2 を全部覆うマッチングと、白の 2 を全部覆う
// マッチングが別々にあれば、両方を同時に覆うマッチングもある。そこで、黒の 2 のマスから 1 でない白のマスへの最大マッチ
// ングと、白の 2 のマスから 1 でない黒のマスへの最大マッチングを Dinic で求め、どちらも 2 のマスを全部覆えば Yes とする
// (下限付きの流れは使わない)。小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
using namespace std;
struct Dinic {
  struct E { int to, rev, cap; };
  vector<vector<E>> g;
  vector<int> lv, it, q;
  explicit Dinic(int n) : g(n), lv(n), it(n), q(n) {}
  void add(int u, int v, int c) {
    g[u].push_back({v, (int)g[v].size(), c});
    g[v].push_back({u, (int)g[u].size() - 1, 0});
  }
  int dfs(int v, int t, int f) {
    if (v == t) return f;
    for (int& i = it[v]; i < (int)g[v].size(); ++i) {
      E& e = g[v][i];
      if (e.cap > 0 && lv[e.to] == lv[v] + 1) {
        const int d = dfs(e.to, t, min(f, e.cap));
        if (d > 0) {
          e.cap -= d, g[e.to][e.rev].cap += d;
          return d;
        }
      }
    }
    return 0;
  }
  int flow(int s, int t) {
    int tot = 0;
    for (;;) {
      fill(lv.begin(), lv.end(), -1);
      int qh = 0, qt = 0;
      lv[s] = 0, q[qt++] = s;
      while (qh < qt) {
        const int u = q[qh++];
        for (auto& e : g[u])
          if (e.cap > 0 && lv[e.to] < 0) lv[e.to] = lv[u] + 1, q[qt++] = e.to;
      }
      if (lv[t] < 0) return tot;
      fill(it.begin(), it.end(), 0);
      for (int d; (d = dfs(s, t, 1 << 30)) > 0;) tot += d;
    }
  }
};
int main() {
  int h, w;
  if (scanf("%d %d", &h, &w) != 2) return 1;
  vector<string> c(h);
  for (auto& row : c) {
    static char buf[512];
    if (scanf("%511s", buf) != 1) return 1;
    row = buf;
  }
  // 色 col の 2 のマスを、1 でない隣のマスで全部覆えるか。
  auto covers = [&](int col) {
    const int S = h * w, T = h * w + 1;
    Dinic d(h * w + 2);
    int need = 0;
    const int di[4] = {1, -1, 0, 0}, dj[4] = {0, 0, 1, -1};
    for (int i = 0; i < h; ++i)
      for (int j = 0; j < w; ++j) {
        if (c[i][j] == '1') continue;
        const int v = i * w + j;
        if ((i + j) % 2 == col) {
          if (c[i][j] != '2') continue;
          d.add(S, v, 1), ++need;
          for (int k = 0; k < 4; ++k) {
            const int a = i + di[k], b = j + dj[k];
            if (0 <= a && a < h && 0 <= b && b < w && c[a][b] != '1') d.add(v, a * w + b, 1);
          }
        } else {
          d.add(v, T, 1);
        }
      }
    return d.flow(S, T) == need;
  };
  puts(covers(0) && covers(1) ? "Yes" : "No");
}
