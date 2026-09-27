// arc080-d の愚直解。カード i と i - 1 の表裏が違う所 (境目) を頂点に見ると、l から p 枚裏返す操作は
// 境目 l と l + p を入れ替える辺になる (l >= 1)。操作の最小回数は、境目どうしを最短路で組にしたときの
// 長さの和の最小 (T-join)。最短路は 1 から x_N + 100 までの位置で幅優先探索して求め、組み方はビット DP で
// 全部試す。差が奇素数なら 1 回、偶数なら 2 回、…という重みを使わず、一般のグラフの重み付きマッチングも
// 解かないので、提出とは別の考え方になる。境目が 22 個まで、x が数千までの小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <queue>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> x(n);
  for (int& v : x)
    if (scanf("%d", &v) != 1) return 1;
  int limit = x.back() + 100;  // 探索する位置は 1 から limit まで
  std::vector<bool> up(limit + 2, false);
  for (int v : x) up[v] = true;
  std::vector<int> ends;  // 境目: up[i] != up[i - 1] の i
  for (int i = 1; i <= limit; ++i)
    if (up[i] != up[i - 1]) ends.push_back(i);
  std::vector<int> primes;  // 3 以上の素数
  for (int p = 3; p <= limit; ++p) {
    bool prime = true;
    for (int d = 2; d * d <= p; ++d)
      if (p % d == 0) prime = false;
    if (prime) primes.push_back(p);
  }
  int k = ends.size();
  const int INF = 1 << 29;
  std::vector<std::vector<int>> dist(k, std::vector<int>(k, INF));
  for (int a = 0; a < k; ++a) {
    std::vector<int> d(limit + 1, INF);
    std::queue<int> bfs;
    d[ends[a]] = 0, bfs.push(ends[a]);
    while (!bfs.empty()) {
      int v = bfs.front();
      bfs.pop();
      for (int p : primes)
        for (int u : {v - p, v + p})
          if (1 <= u && u <= limit && d[u] == INF) d[u] = d[v] + 1, bfs.push(u);
    }
    for (int b = 0; b < k; ++b) dist[a][b] = d[ends[b]];
  }
  // best[mask] = mask の境目を組にし終えたときの最小。まだの境目のうち番号の最も小さいものを誰かと組む。
  std::vector<int> best(1 << k, INF);
  best[0] = 0;
  for (int mask = 0; mask < (1 << k); ++mask) {
    if (best[mask] == INF) continue;
    int a = 0;
    while (a < k && (mask >> a & 1)) ++a;
    if (a == k) continue;
    for (int b = a + 1; b < k; ++b)
      if (!(mask >> b & 1)) {
        int next = mask | 1 << a | 1 << b;
        best[next] = std::min(best[next], best[mask] + dist[a][b]);
      }
  }
  printf("%d\n", best[(1 << k) - 1]);
}
