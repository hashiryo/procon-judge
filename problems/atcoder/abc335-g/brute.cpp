// abc335-g の愚直解。A に出てくる値 a ごとに、a, a^2, a^3, ... を a に戻るまで掛けて、a が生む部分群を 1 周し、
// その中の値を持つ A_j の個数を足す (部分群の元は重ならないので、値ごとの個数の和がそのまま A_j の数になる)。
// 位数も P - 1 の約数も求めないので、OrderFp と約数のゼータ変換を使う提出とは別の考え方になる。
// 値ごとに P に比例する時間がかかるので、P が小さいときだけ使う。
#include <cstdio>
#include <vector>

int main() {
  long long n, p;
  if (scanf("%lld %lld", &n, &p) != 2) return 1;
  std::vector<long long> a(n), count(p);
  for (auto& v : a) {
    if (scanf("%lld", &v) != 1) return 1;
    ++count[v];
  }
  long long pairs = 0;
  for (long long v = 1; v < p; ++v) {
    if (!count[v]) continue;
    long long in_group = 0, x = v;
    do in_group += count[x], x = x * v % p;
    while (x != v);
    pairs += count[v] * in_group;
  }
  printf("%lld\n", pairs);
}
