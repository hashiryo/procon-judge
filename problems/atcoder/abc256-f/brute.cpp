// abc256-f の愚直解。A を配列のまま持って 1 x v では書き換えるだけにし、2 x のたびに B、C、D の累積和を
// 先頭から x 項まで作り直す。区間に作用する木も、係数 (x - j + 1)(x - j + 2) / 2 の式も使わないので、
// 提出とは別の考え方になる。O(NQ) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  const long long P = 998244353;
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<long long> a(n);
  for (auto& v : a)
    if (scanf("%lld", &v) != 1) return 1;
  while (q--) {
    int type, x;
    if (scanf("%d %d", &type, &x) != 2) return 1;
    if (type == 1) {
      if (scanf("%lld", &a[x - 1]) != 1) return 1;
      continue;
    }
    long long b = 0, c = 0, d = 0;
    for (int i = 0; i < x; ++i) {
      b = (b + a[i]) % P;
      c = (c + b) % P;
      d = (d + c) % P;
    }
    printf("%lld\n", d);
  }
}
