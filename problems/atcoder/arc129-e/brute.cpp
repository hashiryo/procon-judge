// arc129-e の愚直解。x_i の選び方 M^N 通りを DFS で全部試し、C の和と |x_i - x_j| * W_{i,j} の和を
// そのまま足して最小を取る。最小カットに直さないので、monge_mincut とは別の考え方になる。
// M^N が小さいときだけ使う。
#include <cstdio>
#include <cstdlib>
#include <vector>

int n, m;
std::vector<std::vector<long long>> a, c, w;
std::vector<long long> x;
long long best = -1;

// i 番目より前は x が決まっていて、それまでの費用が cost。
void dfs(int i, long long cost) {
  if (i == n) {
    if (best < 0 || cost < best) best = cost;
    return;
  }
  for (int k = 0; k < m; ++k) {
    long long add = c[i][k];
    for (int j = 0; j < i; ++j) add += std::llabs(x[j] - a[i][k]) * w[j][i];
    x[i] = a[i][k];
    dfs(i + 1, cost + add);
  }
}

int main() {
  if (scanf("%d %d", &n, &m) != 2) return 1;
  a.assign(n, std::vector<long long>(m));
  c.assign(n, std::vector<long long>(m));
  w.assign(n, std::vector<long long>(n, 0));
  x.assign(n, 0);
  for (int i = 0; i < n; ++i)
    for (int k = 0; k < m; ++k)
      if (scanf("%lld %lld", &a[i][k], &c[i][k]) != 2) return 1;
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j)
      if (scanf("%lld", &w[i][j]) != 1) return 1;
  dfs(0, 0);
  printf("%lld\n", best);
}
