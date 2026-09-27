// abc212-h の愚直解。Nim は xor が 0 でなければ先手の勝ちなので、長さ m の列の xor の値ごとの個数を
// m = 1 から N まで DP で数え、xor が 0 でないものを足す。値は A の xor で作れるもの (span) だけを持つ。
// アダマール変換も等比数列の和の式も使わないので、提出とは別の考え方になる。O(N |span| K) なので、
// 小さい入力でだけ使う。
#include <algorithm>
#include <cstdio>
#include <vector>

int main() {
  const long long P = 998244353;
  int n, k;
  if (scanf("%d %d", &n, &k) != 2) return 1;
  std::vector<int> a(k);
  for (auto &v : a)
    if (scanf("%d", &v) != 1) return 1;
  // span: 0 から始めて A の元との xor で閉じるまで広げる。
  std::vector<int> id(1 << 16, -1), span = {0};
  id[0] = 0;
  for (size_t i = 0; i < span.size(); ++i)
    for (int v : a)
      if (int w = span[i] ^ v; id[w] < 0) id[w] = span.size(), span.push_back(w);
  int s = span.size();
  std::vector<long long> cnt(s, 0), next(s);
  cnt[0] = 1;  // 長さ 0 の列
  long long answer = 0;
  for (int m = 1; m <= n; ++m) {
    std::fill(next.begin(), next.end(), 0);
    for (int i = 0; i < s; ++i)
      if (cnt[i])
        for (int v : a) {
          long long &to = next[id[span[i] ^ v]];
          to = (to + cnt[i]) % P;
        }
    cnt.swap(next);
    for (int i = 1; i < s; ++i) answer = (answer + cnt[i]) % P;
  }
  printf("%lld\n", answer);
}
