// abc222-g の愚直解。a = 2 mod K から始めて a = (10a + 2) mod K を K 回までくり返し、初めて 0 になった項の番号を出す。
// 値は K 通りしかないので、K 項までに 0 が出なければ、そのあとも出ない (-1)。
// 離散対数 (Baby-step Giant-step) を使わないので、提出とは別の考え方になる。O(K) なので、K が小さいときだけ使う。
#include <cstdio>

int main() {
  int t;
  if (scanf("%d", &t) != 1) return 1;
  while (t--) {
    long long k;
    if (scanf("%lld", &k) != 1) return 1;
    long long a = 2 % k, ans = -1;
    for (long long i = 1; i <= k; ++i) {
      if (a == 0) {
        ans = i;
        break;
      }
      a = (a * 10 + 2) % k;
    }
    printf("%lld\n", ans);
  }
}
