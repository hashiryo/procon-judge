// 愚直解。N ≤ 6 の入力で、向きの組 8^N 通りを辞書順に全部試し、時刻 T の位置が B と 1 つずつ一致する最初の組を出す。
#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>
using namespace std;
using i64 = long long;
constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
int main() {
  int n;
  i64 t;
  if (scanf("%d %lld", &n, &t) != 2) return 1;
  vector<pair<i64, i64>> a(n), b(n);
  for (auto& [x, y] : a) if (scanf("%lld %lld", &x, &y) != 2) return 1;
  for (auto& [x, y] : b) if (scanf("%lld %lld", &x, &y) != 2) return 1;
  sort(b.begin(), b.end());
  vector<int> d(n, 0);
  for (;;) {
    vector<pair<i64, i64>> p(n);
    for (int i = 0; i < n; ++i) p[i] = {a[i].first + t * DX[d[i]], a[i].second + t * DY[d[i]]};
    sort(p.begin(), p.end());
    if (p == b) {
      puts("Yes");
      for (int i = 0; i < n; ++i) printf("%d%c", d[i] + 1, i + 1 == n ? '\n' : ' ');
      return 0;
    }
    int k = n - 1;
    while (k >= 0 && d[k] == 7) d[k--] = 0;
    if (k < 0) break;
    ++d[k];
  }
  puts("No");
}
