// past202303-m の愚直解。荷物ごとに箱 1 から順に空きを調べ、最初に入る箱に入れる。問題文の手順そのままで、
// セグメント木の max_right で探さないので、提出とは別の考え方になる。O(NM) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<long long> a(n), room(m), size(m);
  for (auto& x : a)
    if (scanf("%lld", &x) != 1) return 1;
  for (int j = 0; j < m; ++j) {
    if (scanf("%lld", &size[j]) != 1) return 1;
    room[j] = size[j];
  }
  for (int i = 0; i < n; ++i) {
    int j = 0;
    while (j < m && room[j] < a[i]) ++j;
    if (j == m) {
      printf("No\n%d\n", i + 1);
      return 0;
    }
    room[j] -= a[i];
  }
  puts("Yes");
  for (int j = 0; j < m; ++j) printf("%lld%c", size[j] - room[j], j + 1 == m ? '\n' : ' ');
}
