// abc179-d の愚直解。S の要素 d を 1 つずつ並べ、マス i に来る方法の数を、最後に跳んだ d ごとに
// ways[i - d] を足して求める。区間の端だけを見る差分も形式的冪級数の割り算も使わないので、
// 提出とは別の考え方になる。O(N |S|) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n, k;
  if (scanf("%d %d", &n, &k) != 2) return 1;
  std::vector<int> s;
  for (int i = 0; i < k; ++i) {
    int l, r;
    if (scanf("%d %d", &l, &r) != 2) return 1;
    for (int d = l; d <= r; ++d) s.push_back(d);
  }
  const long long mod = 998244353;
  std::vector<long long> ways(n + 1, 0);
  ways[1] = 1;
  for (int i = 2; i <= n; ++i)
    for (int d : s)
      if (d < i) ways[i] = (ways[i] + ways[i - d]) % mod;
  printf("%lld\n", ways[n]);
}
