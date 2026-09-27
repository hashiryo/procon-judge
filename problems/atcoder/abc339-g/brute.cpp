// abc339-g の愚直解。1 つ前の答えで復号し、A_L から A_R までを 1 つずつ見て X 以下のものを足す。
// 区間を木やバケットに分けないので、SegmentTree_2D や SortedPerBucket とは別の考え方になる。
// O(NQ) なので小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> a(n);
  for (auto& v : a)
    if (scanf("%lld", &v) != 1) return 1;
  int q;
  if (scanf("%d", &q) != 1) return 1;
  long long prev = 0;
  for (int k = 0; k < q; ++k) {
    long long alpha, beta, gamma;
    if (scanf("%lld %lld %lld", &alpha, &beta, &gamma) != 3) return 1;
    long long l = alpha ^ prev, r = beta ^ prev, x = gamma ^ prev;
    long long sum = 0;
    for (long long i = l; i <= r; ++i)
      if (a[i - 1] <= x) sum += a[i - 1];
    printf("%lld\n", sum);
    prev = sum;
  }
}
