// abc266-h の愚直解。スヌケを時刻の順に並べ、dp[j] = j を最後に捕まえたときの大きさの和の最大、を
// 前のすべての i (と時刻 0 の原点) について、Y_i <= Y_j かつ |X_j - X_i| + (Y_j - Y_i) <= T_j - T_i
// で移れるかを直接確かめて求める。座標を変換して 2 次元の区間最大を取る、という提出の考え方を使わない。
// O(N^2) なので小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  struct Snuke {
    long long t, x, y, a;
  };
  std::vector<Snuke> s(n);
  for (auto& e : s)
    if (scanf("%lld %lld %lld %lld", &e.t, &e.x, &e.y, &e.a) != 4) return 1;
  std::sort(s.begin(), s.end(), [](const Snuke& l, const Snuke& r) { return l.t < r.t; });
  auto can_move = [](long long t0, long long x0, long long y0, const Snuke& to) {
    return y0 <= to.y && std::llabs(to.x - x0) + (to.y - y0) <= to.t - t0;
  };
  const long long NONE = -1;
  std::vector<long long> dp(n, NONE);
  long long ans = 0;
  for (int j = 0; j < n; ++j) {
    long long best = can_move(0, 0, 0, s[j]) ? 0 : NONE;
    for (int i = 0; i < j; ++i)
      if (dp[i] != NONE && can_move(s[i].t, s[i].x, s[i].y, s[j])) best = std::max(best, dp[i]);
    if (best == NONE) continue;
    dp[j] = best + s[j].a;
    ans = std::max(ans, dp[j]);
  }
  printf("%lld\n", ans);
}
