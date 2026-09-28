// abc327-d の愚直解。0 と 1 の列 X を 2^N 通り全部試し、どれかが全部の i で X_{A_i} ≠ X_{B_i} を
// 満たせば Yes。グラフを幅優先で 2 色に塗らないので、paint_two_colors とは別の考え方になる。
// N が小さいときだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, m;
  if (scanf("%d %d", &n, &m) != 2) return 1;
  std::vector<int> a(m), b(m);
  for (int& x : a)
    if (scanf("%d", &x) != 1) return 1;
  for (int& x : b)
    if (scanf("%d", &x) != 1) return 1;
  bool good = false;
  for (long long mask = 0; mask < (1LL << n) && !good; ++mask) {
    bool ok = true;
    for (int i = 0; i < m && ok; ++i) ok = ((mask >> (a[i] - 1)) & 1) != ((mask >> (b[i] - 1)) & 1);
    good = ok;
  }
  puts(good ? "Yes" : "No");
}
