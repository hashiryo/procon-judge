// 期待出力を作る参照実装。飛行機 i から、時刻 T の位置 A_i + T * (向き d の差分) が B のどれかに当たる向きごとに辺を張り、
// Hopcroft-Karp で最大マッチングを求める。B の点は座標で整列して二分探索で引く。ライブラリを include しない (期待出力の
// キャッシュの鍵はこのファイルの中身だけで決まる)。向きの組は checker.cpp が確かめるので、期待出力で判定に使うのは
// Yes か No かだけ。
#include <algorithm>
#include <array>
#include <cstdio>
#include <queue>
#include <utility>
#include <vector>
using namespace std;
using i64 = long long;
constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
int n;
vector<vector<array<int, 2>>> adj;  // (B の番号, 向き)
vector<int> ml, md, mr, dist_, it_;
bool bfs() {
  queue<int> q;
  bool found = false;
  for (int i = 0; i < n; ++i) {
    dist_[i] = ml[i] < 0 ? 0 : -1;
    if (ml[i] < 0) q.push(i);
  }
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (auto [j, d] : adj[u]) {
      int w = mr[j];
      if (w < 0) found = true;
      else if (dist_[w] < 0) dist_[w] = dist_[u] + 1, q.push(w);
    }
  }
  return found;
}
bool dfs(int u) {
  for (int& k = it_[u]; k < (int)adj[u].size(); ++k) {
    auto [j, d] = adj[u][k];
    int w = mr[j];
    if (w < 0 || (dist_[w] == dist_[u] + 1 && dfs(w))) {
      ml[u] = j, md[u] = d, mr[j] = u;
      return true;
    }
  }
  dist_[u] = -1;
  return false;
}
int main() {
  i64 t;
  if (scanf("%d %lld", &n, &t) != 2) return 1;
  vector<pair<i64, i64>> a(n), b(n);
  for (auto& [x, y] : a) if (scanf("%lld %lld", &x, &y) != 2) return 1;
  for (auto& [x, y] : b) if (scanf("%lld %lld", &x, &y) != 2) return 1;
  vector<int> ord(n);
  for (int j = 0; j < n; ++j) ord[j] = j;
  sort(ord.begin(), ord.end(), [&](int i, int j) { return b[i] < b[j]; });
  vector<pair<i64, i64>> sb(n);
  for (int k = 0; k < n; ++k) sb[k] = b[ord[k]];
  adj.assign(n, {});
  for (int i = 0; i < n; ++i)
    for (int d = 0; d < 8; ++d) {
      pair<i64, i64> q{a[i].first + t * DX[d], a[i].second + t * DY[d]};
      auto p = lower_bound(sb.begin(), sb.end(), q);
      if (p != sb.end() && *p == q) adj[i].push_back({ord[p - sb.begin()], d});
    }
  ml.assign(n, -1), md.assign(n, -1), mr.assign(n, -1), dist_.assign(n, -1), it_.assign(n, 0);
  int size = 0;
  while (bfs()) {
    fill(it_.begin(), it_.end(), 0);
    for (int i = 0; i < n; ++i)
      if (ml[i] < 0 && dfs(i)) ++size;
  }
  if (size < n) return puts("No"), 0;
  puts("Yes");
  for (int i = 0; i < n; ++i) printf("%d%c", md[i] + 1, i + 1 == n ? '\n' : ' ');
}
