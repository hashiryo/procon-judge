// abc256-d の愚直解。長さ 1 のマス [x, x + 1) のうち、どれかの区間に入るものを 1 マスずつ塗り、
// 塗ったマスが続く所を左から順に出す。区間の集合を持って重なりを消していく RangeSet を使わないので、
// 提出とは別の考え方になる。O(区間の長さの和) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  const int MAX_R = 200000;
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<char> painted(MAX_R + 2, 0);  // painted[x] はマス [x, x + 1) が塗られているか
  for (int i = 0; i < n; ++i) {
    int l, r;
    if (scanf("%d %d", &l, &r) != 2) return 1;
    for (int x = l; x < r; ++x) painted[x] = 1;
  }
  for (int x = 1; x <= MAX_R; ++x) {
    if (!painted[x] || painted[x - 1]) continue;
    int y = x;
    while (painted[y]) ++y;
    printf("%d %d\n", x, y);
  }
}
