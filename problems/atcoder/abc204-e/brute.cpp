// abc204-e の愚直解。ダイクストラ法で、町に時刻 d で着いたら、道を渡り始める時刻 t を d から 1 つずつ全部試し、
// 着く時刻 t + C + floor(D / (t + 1)) の最小を整数のまま求める。着く時刻は t + C より早くならないので、
// t + C がそれまでの最小に届いたら打ち切る (試す t は 2 sqrt(D) + 1 個ほど)。待てるので、早く着くほうが悪くなることはない。
// 渡り始める時刻をフィボナッチ探索で探さず、浮動小数点も使わないので、提出とは別の考え方になる。
// 道 1 本ごとに O(sqrt(D)) かかるので、小さい入力でだけ使う。
#include <climits>
#include <cstdio>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  struct Road {
    int to;
    long long c, d;
  };
  std::vector<std::vector<Road>> adj(n + 1);
  for (int i = 0; i < m; ++i) {
    int a, b;
    long long c, d;
    if (scanf("%d %d %lld %lld", &a, &b, &c, &d) != 4) return 1;
    adj[a].push_back({b, c, d}), adj[b].push_back({a, c, d});
  }
  std::vector<long long> dist(n + 1, LLONG_MAX);
  using Item = std::pair<long long, int>;
  std::priority_queue<Item, std::vector<Item>, std::greater<>> pq;
  dist[1] = 0, pq.push({0, 1});
  while (!pq.empty()) {
    auto [now, u] = pq.top();
    pq.pop();
    if (now != dist[u]) continue;
    for (const Road& r : adj[u]) {
      long long best = LLONG_MAX;
      for (long long t = now; t + r.c < best; ++t) best = std::min(best, t + r.c + r.d / (t + 1));
      if (best < dist[r.to]) dist[r.to] = best, pq.push({best, r.to});
    }
  }
  printf("%lld\n", dist[n] == LLONG_MAX ? -1 : dist[n]);
}
