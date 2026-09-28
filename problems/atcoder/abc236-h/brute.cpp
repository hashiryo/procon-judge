// abc236-h の愚直解。A_1, A_2, ... の順に D_i の倍数 D_i, 2 D_i, ..., を 1 つずつ置き、前に置いたものと
// 同じならそこで打ち切る。最後まで置けた並びを数える。最小公倍数での包除 (sps::exp) を使わないので、
// 提出とは別の考え方になる。floor(M / D_i) の積が小さい入力でだけ使う。
#include <cstdio>
#include <vector>

using u64 = unsigned long long;
int n;
u64 m;
std::vector<u64> d, chosen;

u64 count(int i) {
  if (i == n) return 1;
  u64 total = 0;
  for (u64 a = d[i]; a <= m; a += d[i]) {
    bool used = false;
    for (int j = 0; j < i && !used; ++j) used = chosen[j] == a;
    if (used) continue;
    chosen[i] = a;
    total += count(i + 1);
  }  // a + D_i は 2 × 10^18 以下なので 2^64 を超えない
  return total;
}

int main() {
  if (scanf("%d %llu", &n, &m) != 2) return 1;
  d.resize(n), chosen.resize(n);
  for (auto &x : d)
    if (scanf("%llu", &x) != 1) return 1;
  printf("%llu\n", count(0) % 998244353);
}
