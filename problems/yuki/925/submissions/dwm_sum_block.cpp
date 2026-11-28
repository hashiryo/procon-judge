// yukicoder No.925 紲星 Extra。点更新と区間中央値・絶対偏差和。
// 重み付きの動的 wavelet matrix (256 bit の塊 + Fenwick、各 bit に値の重みを持つ) で、
// k 番目、x 未満の個数と重みの和、区間の重みの和を返す。
#include <algorithm>
#include <cstdio>
#include <vector>
#include "_shared/wavelet/dwm_sum.hpp"

using namespace std;
using namespace dwm_sum_internal;

signed main() {
  int N, Q;
  if (scanf("%d %d", &N, &Q) != 2) return 1;
  vector<long long> a(N);
  for (auto &x : a) scanf("%lld", &x);
  WeightedWaveletMatrix<40> wm(a);
  long long s = 0;
  while (Q--) {
    int op;
    scanf("%d", &op);
    if (op == 1) {
      long long X, Y;
      scanf("%lld %lld", &X, &Y);
      (X ^= s) &= (1ll << 16) - 1;
      (Y ^= s) &= (1ll << 40) - 1;
      --X;
      wm.set((int)X, Y);
    } else {
      long long L, R;
      scanf("%lld %lld", &L, &R);
      (L ^= s) &= (1ll << 16) - 1;
      (R ^= s) &= (1ll << 16) - 1;
      if (L > R) swap(L, R);
      --L;
      int len = (int)(R - L);
      long long x = wm.kth((int)L, (int)R, len / 2);
      long long total = wm.sum_all((int)L, (int)R);
      long long slt = wm.sum_lt((int)L, (int)R, x);
      int clt = wm.count_lt((int)L, (int)R, x);
      long long ans = total - 2 * slt - x * (len - 2 * clt);
      printf("%lld\n", ans);
      s ^= ans;
    }
  }
  return 0;
}
