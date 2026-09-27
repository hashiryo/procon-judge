// arc097-d の愚直解。(猫のいる頂点, 全頂点の色) を状態にして、全部の頂点を始点に幅優先探索し、
// 全部黒の状態に初めて着いた手数を出す。部分木ごとの値を持ち直す Rerooting をしないので、
// 提出とは別の考え方になる。状態が N 2^N 個あるので、小さい木でだけ使う。
#include <cstdio>
#include <queue>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<std::vector<int>> adj(n);
  for (int i = 0; i < n - 1; ++i) {
    int x, y;
    if (scanf("%d %d", &x, &y) != 2) return 1;
    --x, --y;
    adj[x].push_back(y), adj[y].push_back(x);
  }
  static char c[32];
  if (scanf("%31s", c) != 1) return 1;
  int white = 0;  // bit v が 1 なら頂点 v は白
  for (int v = 0; v < n; ++v)
    if (c[v] == 'W') white |= 1 << v;
  if (white == 0) return puts("0"), 0;
  std::vector<int> dist(n << n, -1);
  std::queue<int> bfs;  // 状態は (色 << 5) | 位置
  auto push = [&](int v, int mask, int d) {
    int s = mask * n + v;
    if (dist[s] < 0) dist[s] = d, bfs.push(mask << 5 | v);
  };
  for (int v = 0; v < n; ++v) push(v, white, 0);
  while (!bfs.empty()) {
    int v = bfs.front() & 31, mask = bfs.front() >> 5;
    bfs.pop();
    int d = dist[mask * n + v];
    if (mask == 0) return printf("%d\n", d), 0;
    push(v, mask ^ 1 << v, d + 1);  // 今いる頂点の色を変える
    for (int u : adj[v]) push(u, mask ^ 1 << u, d + 1);  // 隣へ動き、行き先の色を変える
  }
  return 1;
}
