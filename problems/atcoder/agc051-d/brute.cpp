// agc051-d の愚直解。今いる頂点と、4 本の辺それぞれの残りの通る回数を状態にして、S に戻る歩き方を
// メモ化で数える (残りが全部 0 で S にいれば 1 通り)。オイラー閉路の BEST 定理も行列式も使わないので、
// 提出とは別の考え方になる。状態が 4 (a+1)(b+1)(c+1)(d+1) 個なので、値が小さいときだけ使う。
#include <cstdio>
#include <vector>

using u32 = unsigned;
constexpr u32 P = 998244353, NONE = 0xffffffffu;

int lim[4];
std::vector<u32> memo;

// 頂点は S = 0, T = 1, U = 2, V = 3。辺 e は頂点 e と e + 1 (mod 4) を結ぶ (ST, TU, UV, VS)。
u32 ways(int v, int r[4]) {
  size_t key = v;
  for (int e = 0; e < 4; ++e) key = key * (lim[e] + 1) + r[e];
  if (memo[key] != NONE) return memo[key];
  u32 total = 0;
  if (r[0] == 0 && r[1] == 0 && r[2] == 0 && r[3] == 0) total = v == 0;
  // 頂点 v から出る辺は、辺 v (v + 1 へ) と辺 v - 1 (v - 1 へ)。
  for (int e : {v, (v + 3) % 4})
    if (r[e] > 0) {
      int to = e == v ? (v + 1) % 4 : e;
      --r[e];
      total = (total + ways(to, r)) % P;
      ++r[e];
    }
  return memo[key] = total;
}

int main() {
  if (scanf("%d %d %d %d", &lim[0], &lim[1], &lim[2], &lim[3]) != 4) return 1;
  memo.assign(4ULL * (lim[0] + 1) * (lim[1] + 1) * (lim[2] + 1) * (lim[3] + 1), NONE);
  int r[4] = {lim[0], lim[1], lim[2], lim[3]};
  printf("%u\n", ways(0, r));
}
