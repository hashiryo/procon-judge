// 愚直解。行の選び方と列の選び方を全部試し、負のマスを行と列の両方で選ばないもののうち、行か列で選んだマスの和の最大を取る。
// H + W <= 16 ほどまで。
#include <cstdio>
#include <vector>
using namespace std;
int main() {
  int h, w;
  if (scanf("%d %d", &h, &w) != 2) return 1;
  vector<vector<long long>> a(h, vector<long long>(w));
  for (auto& row : a)
    for (auto& x : row)
      if (scanf("%lld", &x) != 1) return 1;
  long long best = 0;
  for (int rm = 0; rm < (1 << h); ++rm)
    for (int cm = 0; cm < (1 << w); ++cm) {
      bool ok = true;
      long long s = 0;
      for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c) {
          const bool br = rm >> r & 1, bc = cm >> c & 1;
          if (br && bc && a[r][c] < 0) ok = false;
          if (br || bc) s += a[r][c];
        }
      if (ok && s > best) best = s;
    }
  printf("%lld\n", best);
}
