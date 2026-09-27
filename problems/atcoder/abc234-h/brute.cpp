// abc234-h の愚直解。p < q の組を全部、p の小さい順、同じ p なら q の小さい順に調べ、
// (x_p - x_q)^2 + (y_p - y_q)^2 <= K^2 を 64 ビットの整数で確かめて出す。KDTree も格子への振り分けもしないので、
// 提出とは別の考え方になる。O(N^2) だが中の比べ方が単純なので、N = 2 × 10^5 でも数十秒で終わる。
#include <cstdio>
#include <string>
#include <vector>

int main() {
  int n;
  long long k;
  if (scanf("%d %lld", &n, &k) != 2) return 1;
  std::vector<long long> x(n), y(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %lld", &x[i], &y[i]) != 2) return 1;
  const long long k2 = k * k;  // 2.25 × 10^18 まで
  std::vector<std::pair<int, int>> pairs;
  for (int p = 0; p < n; ++p)
    for (int q = p + 1; q < n; ++q) {
      long long dx = x[p] - x[q], dy = y[p] - y[q];
      if (dx * dx + dy * dy <= k2) pairs.emplace_back(p + 1, q + 1);  // 2 × 10^18 まで
    }
  std::string out = std::to_string(pairs.size()) + "\n";
  for (auto [p, q] : pairs) out += std::to_string(p) + " " + std::to_string(q) + "\n";
  fputs(out.c_str(), stdout);
}
