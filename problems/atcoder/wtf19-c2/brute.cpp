// wtf19-c2 の愚直解。一番上の行から順に、点いているランプ (x, y) に (x, y - 1) での操作をして、
// (x, y) を消し (x, y - 1) と (x + 1, y - 1) を切り替える。全部を一番下の行 y0 まで押し出すと、
// 元のランプ (X, Y) を押し出したものと同じ並び、つまり x = X + j のうち C(Y - y0, j) が奇数の所だけが
// 点いた行になるので、左端から X を、幅から Y を読む (Y >= y0 であることは問題から言える)。
// 読んだ (X, Y) を押し出した並びと一致するかも確かめる。Nimber の離散対数を使わないので、
// 提出とは別の考え方になる。広がりが小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<long long> xs(n), ys(n);
  for (int i = 0; i < n; ++i)
    if (scanf("%lld %lld", &xs[i], &ys[i]) != 2) return 1;
  long long x0 = *std::min_element(xs.begin(), xs.end()), x1 = *std::max_element(xs.begin(), xs.end());
  long long y0 = *std::min_element(ys.begin(), ys.end()), y1 = *std::max_element(ys.begin(), ys.end());
  long long h = y1 - y0 + 1, w = (x1 - x0) + (y1 - y0) + 1;
  if (h > 20000 || w > 40000 || h * w > 200000000LL) return 1;
  std::vector<std::vector<char>> on(h, std::vector<char>(w, 0));
  for (int i = 0; i < n; ++i) on[ys[i] - y0][xs[i] - x0] ^= 1;
  for (long long y = h - 1; y >= 1; --y)
    for (long long x = 0; x + 1 < w; ++x)
      if (on[y][x]) on[y][x] = 0, on[y - 1][x] ^= 1, on[y - 1][x + 1] ^= 1;
  long long lo = -1, hi = -1;
  for (long long x = 0; x < w; ++x)
    if (on[0][x]) hi = x, lo = lo < 0 ? x : lo;
  if (lo < 0) return 1;
  long long m = hi - lo;
  for (long long j = 0; lo + j < w; ++j)
    if (on[0][lo + j] != (j <= m && (j & m) == j)) return 1;  // 1 つのランプを押し出した形になっていない
  printf("%lld %lld\n", x0 + lo, y0 + m);
}
