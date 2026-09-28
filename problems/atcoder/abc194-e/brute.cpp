// abc194-e の愚直解。長さ M の区間ごとに、出てきた値に印を付けて 0 から順に探して mex を求め、最小をとる。
// 区間をずらしながら RangeSet で値の集合を持つことはしないので、提出とは別の考え方になる。
// O(N^2) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<int> a(n);
  for (int& x : a)
    if (scanf("%d", &x) != 1) return 1;
  int best = n + 1;
  for (int i = 0; i + m <= n; ++i) {
    std::vector<bool> seen(n + 1, false);  // 値は N 未満なので、seen[N] は立たない
    for (int j = i; j < i + m; ++j) seen[a[j]] = true;
    int mex = 0;
    while (seen[mex]) ++mex;
    if (mex < best) best = mex;
  }
  printf("%d\n", best);
}
