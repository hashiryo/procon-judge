// past202004-n の愚直解。点ごとに正方形を全部見て、xmin <= A <= xmin + D かつ ymin <= B <= ymin + D なら C を足す。
// 点の KDTree に正方形ごとの加算を遅延で配らないので、提出とは別の考え方になる。O(NQ)。
#include <cstdio>
#include <vector>

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  // 座標は ±10^9、xmin + D は 2 × 10^9 までなので long long で持つ。
  std::vector<long long> x1(n), x2(n), y1(n), y2(n), c(n);
  for (int i = 0; i < n; ++i) {
    long long d;
    if (scanf("%lld %lld %lld %lld", &x1[i], &y1[i], &d, &c[i]) != 4) return 1;
    x2[i] = x1[i] + d, y2[i] = y1[i] + d;
  }
  for (int j = 0; j < q; ++j) {
    long long a, b;
    if (scanf("%lld %lld", &a, &b) != 2) return 1;
    long long sum = 0;
    // 分岐しない形にして、制約いっぱいのケース (5 × 10^9 回の判定) も 10 秒ほどで終わるようにする。
    for (int i = 0; i < n; ++i) sum += (x1[i] <= a & a <= x2[i] & y1[i] <= b & b <= y2[i]) ? c[i] : 0;
    printf("%lld\n", sum);
  }
}
