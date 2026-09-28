// abc275-h の愚直解。残りの体力の組 (0 より下は 0 にそろえる) を状態にし、区間 [l, r] を 1 回選ぶ操作を
// 重み max(B_l, ..., B_r) の辺として、最初の体力から全部 0 までの最短路をダイクストラ法で求める。
// デカルト木で分けることも、区分線形凸関数を使うこともしないので、提出とは別の考え方になる。
// 状態は (A_1 + 1)(A_2 + 1)...(A_N + 1) 個なので、N と A が小さいときだけ使う。
#include <algorithm>
#include <cstdio>
#include <functional>
#include <queue>
#include <vector>

using i64 = long long;

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<int> a(n);
  std::vector<i64> b(n);
  for (auto& x : a)
    if (scanf("%d", &x) != 1) return 1;
  for (auto& x : b)
    if (scanf("%lld", &x) != 1) return 1;
  // 状態 = 残りの体力を A_i + 1 進で並べた数。
  std::vector<int> radix(n + 1, 1);
  for (int i = 0; i < n; ++i) radix[i + 1] = radix[i] * (a[i] + 1);
  int start = 0;
  for (int i = 0; i < n; ++i) start += a[i] * radix[i];
  std::vector<i64> dist(radix[n], -1);
  std::priority_queue<std::pair<i64, int>, std::vector<std::pair<i64, int>>, std::greater<>> pq;
  pq.push({0, start});
  while (!pq.empty()) {
    auto [d, s] = pq.top();
    pq.pop();
    if (dist[s] >= 0) continue;
    dist[s] = d;
    if (s == 0) break;
    for (int l = 0; l < n; ++l) {
      i64 cost = 0;
      int t = s;
      for (int r = l; r < n; ++r) {
        cost = std::max(cost, b[r]);
        if (t / radix[r] % (a[r] + 1) > 0) t -= radix[r];  // 位置 r の体力を 1 減らす
        if (dist[t] < 0) pq.push({d + cost, t});
      }
    }
  }
  printf("%lld\n", dist[0]);
}
