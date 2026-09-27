// abc228-d の愚直解。長さ 2^20 の配列を持ち、問題文のとおり h を 1 つずつ進めて空きを探す。
// 埋まった区間の集合を持たないので、RangeSet を使う提出とは別の考え方になる。
// 1 回の書き込みで塊の長さだけ進むので、塊が大きくなる入力は小さい Q でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  constexpr long long N = 1 << 20;
  int q;
  if (scanf("%d", &q) != 1) return 1;
  std::vector<long long> a(N, -1);
  while (q--) {
    int t;
    long long x;
    if (scanf("%d %lld", &t, &x) != 2) return 1;
    if (t == 1) {
      long long h = x;
      while (a[h % N] != -1) ++h;
      a[h % N] = x;
    } else {
      printf("%lld\n", a[x % N]);
    }
  }
}
