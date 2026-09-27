// abc182-e の愚直解。電球ごとに上下左右へ 1 マスずつ進み、ブロックか盤の端に着くまでのマスに印を付ける。
// 行と列をブロックで区間に切る RangeSet を使わないので、提出とは別の考え方になる。O(HW + N(H + W)) なので、
// 電球が多い大きい盤面では遅い。
#include <cstdio>
#include <vector>

int main() {
  int h, w, n, m;
  if (scanf("%d %d %d %d", &h, &w, &n, &m) != 4) return 1;
  std::vector<int> a(n), b(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%d %d", &a[i], &b[i]) != 2) return 1;
  // 0: 空き、1: ブロック
  std::vector<std::vector<char>> block(h + 2, std::vector<char>(w + 2, 0)), lit(h + 2, std::vector<char>(w + 2, 0));
  for (int i = 0; i < m; ++i) {
    int c, d;
    if (scanf("%d %d", &c, &d) != 2) return 1;
    block[c][d] = 1;
  }
  const int di[4] = {-1, 1, 0, 0}, dj[4] = {0, 0, -1, 1};
  for (int k = 0; k < n; ++k) {
    lit[a[k]][b[k]] = 1;
    for (int dir = 0; dir < 4; ++dir)
      for (int i = a[k] + di[dir], j = b[k] + dj[dir]; 1 <= i && i <= h && 1 <= j && j <= w && !block[i][j];
           i += di[dir], j += dj[dir])
        lit[i][j] = 1;
  }
  long long ans = 0;
  for (int i = 1; i <= h; ++i)
    for (int j = 1; j <= w; ++j) ans += lit[i][j] && !block[i][j];
  printf("%lld\n", ans);
}
