// abc117-d の愚直解。X を 0 から K まで全部試して、(X xor A_1) + ... + (X xor A_N) の最大を取る。
// K の 2 進表記をたどる桁 DP (Automaton) を使わないので、提出とは別の考え方になる。O(NK) なので、K が小さいときだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long n, k;
  if (scanf("%lld %lld", &n, &k) != 2) return 1;
  std::vector<long long> a(n);
  for (auto& v : a)
    if (scanf("%lld", &v) != 1) return 1;
  long long best = -1;
  for (long long x = 0; x <= k; ++x) {
    long long f = 0;
    for (long long v : a) f += x ^ v;
    if (f > best) best = f;
  }
  printf("%lld\n", best);
}
