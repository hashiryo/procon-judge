// abc174-f の愚直解。区間ごとに l から r まで色を 1 つずつ見て、初めて見た色を数える。
// 前に同じ色が出た位置を数える WaveletMatrix を使わないので、提出とは別の考え方になる。
// O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<int> c(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%d", &c[i]) != 1) return 1;
  std::vector<int> seen(n + 1, -1);  // seen[色] = その色を最後に見た質問の番号
  for (int t = 0; t < q; ++t) {
    int l, r;
    if (scanf("%d %d", &l, &r) != 2) return 1;
    int count = 0;
    for (int i = l; i <= r; ++i)
      if (seen[c[i]] != t) seen[c[i]] = t, ++count;
    printf("%d\n", count);
  }
}
