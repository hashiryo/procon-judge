// abc274-h の愚直解。質問ごとに S(A(a, b), A(c, d)) と A(e, f) を先頭から 1 つずつ比べ、最初に違う所で
// 大小を決める。違う所が無ければ長さで決める。ハッシュも二分探索も使わないので、RollingHash とは
// 別の考え方になる。共通接頭辞の長さに比例する時間がかかる。
#include <algorithm>
#include <cstdio>
#include <vector>

using u64 = unsigned long long;

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 1;
  std::vector<u64> a(n + 1);
  for (int i = 1; i <= n; ++i)
    if (scanf("%llu", &a[i]) != 1) return 1;
  while (q--) {
    int qa, qb, qc, qd, qe, qf;
    if (scanf("%d %d %d %d %d %d", &qa, &qb, &qc, &qd, &qe, &qf) != 6) return 1;
    int len_s = qb - qa + 1, len_t = qf - qe + 1, common = std::min(len_s, len_t);
    bool smaller = len_s < len_t;  // 最後まで一致したとき
    for (int i = 0; i < common; ++i) {
      u64 s = a[qa + i] ^ a[qc + i], t = a[qe + i];
      if (s != t) {
        smaller = s < t;
        break;
      }
    }
    puts(smaller ? "Yes" : "No");
  }
}
