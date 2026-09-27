// abc214-e の愚直解。Hall の定理で判定する。玉の区間は数直線の区間なので、窓 [a, b] ごとに
// 「[a, b] に収まる区間の数 ≤ b - a + 1」を確かめれば足りる (a はどれかの L、b はどれかの R としてよい)。
// 箱を小さい順に埋める貪欲をしないので、提出とは別の考え方になる。O(N^3) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int t;
  if (scanf("%d", &t) != 1) return 1;
  while (t--) {
    int n;
    if (scanf("%d", &n) != 1) return 1;
    std::vector<long long> l(n), r(n);
    for (int i = 0; i < n; ++i)
      if (scanf("%lld %lld", &l[i], &r[i]) != 2) return 1;
    bool ok = true;
    for (int i = 0; i < n && ok; ++i)
      for (int j = 0; j < n && ok; ++j) {
        long long a = l[i], b = r[j];
        if (a > b) continue;
        long long inside = 0;
        for (int k = 0; k < n; ++k) inside += a <= l[k] && r[k] <= b;
        if (inside > b - a + 1) ok = false;
      }
    puts(ok ? "Yes" : "No");
  }
}
