// abc135-f の愚直解。s をくり返した無限の文字列の始まりの位置 o (0 <= o < |s|) ごとに、o から t を i 回つなげたものが
// 現れるかを i = 1, 2, ... と 1 文字ずつ比べて伸ばす。どこかの o で i 回つなげられる最大の i が答え。t^i の長さが
// |s| + |t| 以上になっても現れるなら、Fine と Wilf の定理から t^i は周期 gcd(|s|, |t|) を持ち、いくらでも伸ばせるので -1。
// ハッシュも位置のグラフも使わないので、提出とは別の考え方になる。O(|s| (|s| + |t|)) なので、短い文字列でだけ使う。
#include <iostream>
#include <string>
#include <vector>

int main() {
  std::string s, t;
  if (!(std::cin >> s >> t)) return 1;
  const long long n = s.size(), m = t.size();
  std::vector<char> alive(n, 1);  // alive[o]: o から t を i 回つなげたものが現れる
  long long best = 0;
  for (long long i = 1;; ++i) {
    bool any = false;
    for (long long o = 0; o < n; ++o) {
      if (!alive[o]) continue;
      for (long long j = 0; j < m; ++j)  // i 個目の t を比べる
        if (s[(o + (i - 1) * m + j) % n] != t[j]) {
          alive[o] = 0;
          break;
        }
      any |= alive[o];
    }
    if (!any) break;
    best = i;
    if (i * m >= n + m) {
      puts("-1");
      return 0;
    }
  }
  printf("%lld\n", best);
}
