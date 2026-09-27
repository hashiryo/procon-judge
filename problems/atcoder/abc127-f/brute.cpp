// abc127-f の愚直解。更新で来た a を全部覚えておき、評価のたびに f を折れ点 (a の値) の全部で計算して、
// 最小値と、それを取る最小の x を選ぶ。f は折れ線なので、最小にする x の最小は折れ点のどれかになる。
// 傾きを平衡二分木で持たないので、PiecewiseLinearConvex とは別の考え方になる。
// 評価 1 回が O(k log k) (k は更新の回数) なので、小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int q;
  if (scanf("%d", &q) != 1) return 1;
  std::vector<long long> a;
  long long b_sum = 0;
  while (q--) {
    int op;
    if (scanf("%d", &op) != 1) return 1;
    if (op == 1) {
      long long x, b;
      if (scanf("%lld %lld", &x, &b) != 2) return 1;
      a.push_back(x), b_sum += b;
      continue;
    }
    std::vector<long long> s = a;
    std::sort(s.begin(), s.end());
    int k = s.size();
    std::vector<long long> prefix(k + 1, 0);
    for (int i = 0; i < k; ++i) prefix[i + 1] = prefix[i] + s[i];
    long long best_x = 0, best = 0;
    for (int j = 0; j < k; ++j) {
      // f(s[j]) = sum |s[j] - s[i]| + b_sum。i <= j と i > j に分けて足す。
      long long left = s[j] * (j + 1) - prefix[j + 1];
      long long right = (prefix[k] - prefix[j + 1]) - s[j] * (k - j - 1);
      long long f = left + right + b_sum;
      if (j == 0 || f < best) best = f, best_x = s[j];
    }
    printf("%lld %lld\n", best_x, best);
  }
}
