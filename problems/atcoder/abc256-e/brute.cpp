// abc256-e の愚直解。配る順 P (順列) を N! 通り全部試し、X_i に i より先に配ったときだけ C_i を足して、
// 最小を取る。閉路を探して閉路ごとの最小を足すことはしないので、強連結成分を使う提出とは別の考え方になる。
// 小さい N でだけ使う。
#include <algorithm>
#include <cstdio>
#include <numeric>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1 || n > 10) return 1;
  std::vector<int> x(n);
  std::vector<long long> c(n);
  for (auto &v : x)
    if (scanf("%d", &v) != 1) return 1;
  for (auto &v : c)
    if (scanf("%lld", &v) != 1) return 1;
  std::vector<int> p(n), when(n);
  std::iota(p.begin(), p.end(), 0);
  long long best = -1;
  do {
    for (int k = 0; k < n; ++k) when[p[k]] = k;  // 人 p[k] に k 番目に配る
    long long sum = 0;
    for (int i = 0; i < n; ++i)
      if (when[x[i] - 1] < when[i]) sum += c[i];
    if (best < 0 || sum < best) best = sum;
  } while (std::next_permutation(p.begin(), p.end()));
  printf("%lld\n", best);
}
