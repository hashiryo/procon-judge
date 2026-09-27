// abc279-d の愚直解。f(n) = B n + A / √(n + 1) を n について 1 つずつ試し、最小を取る。
// A / B が 10^8 までなら n = 0 から、B n がそれまでの最小を超えるまで全部試す (f(n) >= B n なので以後は要らない)。
// それより大きいときは、f が凸で整数での最小が実数の最小 n + 1 = (A / 2B)^(2/3) の両隣にあることを使い、
// その前後 10^6 個を全部試す。探索の区間を縮めていかないので、fibonacci_search を使う提出とは別の考え方になる。
#include <algorithm>
#include <cmath>
#include <cstdio>

int main() {
  long long a, b;
  if (scanf("%lld %lld", &a, &b) != 2) return 1;
  auto f = [&](long long n) { return (long double)b * n + a / std::sqrt((long double)(n + 1)); };
  long double best = f(0);
  if (a / b <= 100000000) {
    for (long long n = 1; (long double)b * n < best; ++n) best = std::min(best, f(n));
  } else {
    long long center = (long long)std::pow((long double)a / (2.0L * b), 2.0L / 3.0L) - 1;
    for (long long n = std::max(0LL, center - 1000000); n <= center + 1000000; ++n) best = std::min(best, f(n));
  }
  printf("%.12Lf\n", best);
}
