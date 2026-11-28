// 期待出力を作る参照実装。木をオイラーツアーの括弧列 (各頂点を開きと閉じで 2 回並べた長さ 2n の列) に落とし、
// パス u-v をその区間に対応させて Mo's algorithm で相異なる色の数を数える。パスに LCA が入らない形の
// 区間では、答えるときだけ LCA の色を足して外す。LCA は二分探索。深さ 1e5 のパスがあるので DFS は
// 再帰で書かず、スタックを持つ。ライブラリは include しない。
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <vector>
using namespace std;

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  vector<int> col(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%d", &col[i]) != 1) return 1;
  // 色を 0 から始まる番号に詰める。
  vector<int> vals = col;
  sort(vals.begin(), vals.end());
  vals.erase(unique(vals.begin(), vals.end()), vals.end());
  const int kinds = (int)vals.size();
  for (int &x : col) x = (int)(lower_bound(vals.begin(), vals.end(), x) - vals.begin());
  // 辺を CSR の隣接にする。
  vector<int> eu(max(n - 1, 0)), ev(max(n - 1, 0)), deg(n, 0);
  for (int i = 0; i < n - 1; ++i) {
    if (scanf("%d %d", &eu[i], &ev[i]) != 2) return 1;
    --eu[i], --ev[i];
    ++deg[eu[i]], ++deg[ev[i]];
  }
  vector<int> start(n + 1, 0);
  for (int i = 0; i < n; ++i) start[i + 1] = start[i] + deg[i];
  vector<int> adj(start[n]), it(start);
  for (int i = 0; i < n - 1; ++i) {
    adj[it[eu[i]]++] = ev[i];
    adj[it[ev[i]]++] = eu[i];
  }
  // 括弧列を作る。in[v] は開き、out[v] は閉じの位置。
  vector<int> in(n), out(n), dep(n), par(n), tour;
  tour.reserve(2 * n);
  {
    vector<int> cur = start, st;
    st.reserve(n);
    in[0] = 0, par[0] = 0, dep[0] = 0;
    tour.push_back(0);
    st.push_back(0);
    while (!st.empty()) {
      const int v = st.back();
      if (cur[v] < start[v + 1]) {
        const int u = adj[cur[v]++];
        if (u == par[v]) continue;
        par[u] = v, dep[u] = dep[v] + 1;
        in[u] = (int)tour.size();
        tour.push_back(u);
        st.push_back(u);
      } else {
        out[v] = (int)tour.size();
        tour.push_back(v);
        st.pop_back();
      }
    }
  }
  const int len = (int)tour.size();  // 2n
  // 二分探索の LCA。
  int lg = 1;
  while ((1 << lg) <= n) ++lg;
  vector<vector<int>> up(lg, vector<int>(n));
  for (int v = 0; v < n; ++v) up[0][v] = par[v];
  for (int k = 1; k < lg; ++k)
    for (int v = 0; v < n; ++v) up[k][v] = up[k - 1][up[k - 1][v]];
  auto lca = [&](int u, int v) {
    if (dep[u] < dep[v]) swap(u, v);
    int d = dep[u] - dep[v];
    for (int k = 0; d; ++k, d >>= 1)
      if (d & 1) u = up[k][u];
    if (u == v) return u;
    for (int k = lg - 1; k >= 0; --k)
      if (up[k][u] != up[k][v]) u = up[k][u], v = up[k][v];
    return up[0][u];
  };
  // クエリを区間 [l, r) と、足す LCA (不要なら -1) に直す。
  vector<int> ql(q), qr(q), extra(q);
  for (int i = 0; i < q; ++i) {
    int u, v;
    if (scanf("%d %d", &u, &v) != 2) return 1;
    --u, --v;
    if (in[u] > in[v]) swap(u, v);
    const int w = lca(u, v);
    if (w == u) ql[i] = in[u], qr[i] = in[v] + 1, extra[i] = -1;
    else ql[i] = out[u], qr[i] = in[v] + 1, extra[i] = w;
  }
  // Mo's algorithm。l のブロックを左から、同じブロックの中は r を昇順と降順の交互に並べる。
  int bs = len / max(1, (int)sqrt((double)q));
  if (bs < 1) bs = 1;
  vector<int> ord(q), blk(q);
  iota(ord.begin(), ord.end(), 0);
  for (int i = 0; i < q; ++i) blk[i] = ql[i] / bs;
  sort(ord.begin(), ord.end(), [&](int a, int b) {
    if (blk[a] != blk[b]) return blk[a] < blk[b];
    return (blk[a] & 1) ? qr[a] > qr[b] : qr[a] < qr[b];
  });
  vector<int> cnt(kinds, 0), ans(q);
  vector<char> on(n, 0);
  int distinct = 0, l = 0, r = 0;
  auto toggle = [&](int p) {
    const int v = tour[p];
    if (on[v]) {
      if (--cnt[col[v]] == 0) --distinct;
    } else {
      if (cnt[col[v]]++ == 0) ++distinct;
    }
    on[v] ^= 1;
  };
  for (int id : ord) {
    while (l > ql[id]) toggle(--l);
    while (r < qr[id]) toggle(r++);
    while (l < ql[id]) toggle(l++);
    while (r > qr[id]) toggle(--r);
    const int w = extra[id];
    if (w >= 0 && !on[w]) {
      if (cnt[col[w]]++ == 0) ++distinct;
      ans[id] = distinct;
      if (--cnt[col[w]] == 0) --distinct;
    } else {
      ans[id] = distinct;
    }
  }
  for (int i = 0; i < q; ++i) printf("%d\n", ans[i]);
}
