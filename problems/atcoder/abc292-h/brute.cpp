// abc292-h の愚直解。変更のたびに、コンテストを前から順にたどって和を足し、和 >= B * n になったところで止め、
// 和 / n を整数の割り算で小数点以下 18 桁まで正確に出す。セグメント木の max_right で届く位置を探さず、
// double も使わないので、提出とは別の考え方になる。O(NQ) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long n, b, q;
  if (scanf("%lld %lld %lld", &n, &b, &q) != 3) return 1;
  std::vector<long long> a(n);
  for (auto& v : a)
    if (scanf("%lld", &v) != 1) return 1;
  while (q--) {
    long long c, x;
    if (scanf("%lld %lld", &c, &x) != 2) return 1;
    a[c - 1] = x;
    long long total = 0, count = 0;
    for (long long v : a) {
      total += v, ++count;
      if (total >= b * count) break;
    }
    long long whole = total / count, rest = total % count;
    printf("%lld.", whole);
    for (int digit = 0; digit < 18; ++digit) rest *= 10, putchar('0' + rest / count), rest %= count;
    putchar('\n');
  }
}
