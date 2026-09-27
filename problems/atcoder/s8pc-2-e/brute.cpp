// s8pc-2-e の愚直解。部分文字列を、いちばん左に現れる位置で数える。位置 i から始まる長さ L の部分文字列が
// 初めて現れるのは、i より前のどの j とも LCP(i, j) < L のときなので、m = max_{j<i} LCP(i, j) として
// 長さ m+1 から N-i までを足す。LCP は後ろの位置から lcp[i][j] = (S[i] == S[j] ? lcp[i+1][j+1] + 1 : 0) で
// 1 行ずつ求める。接尾辞配列を作らないので、提出とは別の考え方になる。O(N^2) なので、短い文字列で使う。
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

int main() {
  static char buf[200005];
  if (scanf("%200004s", buf) != 1) return 1;
  std::string s = buf;
  long long n = s.size(), total = 0;
  // next[j] = LCP(i + 1, j)、cur[j] = LCP(i, j) (j < i)。
  std::vector<int> next(n + 1, 0), cur(n + 1, 0);
  for (long long i = n - 1; i >= 0; --i) {
    int m = 0;
    for (long long j = 0; j < i; ++j) {
      cur[j] = s[i] == s[j] ? next[j + 1] + 1 : 0;
      m = std::max(m, cur[j]);
    }
    long long len = n - i;
    total += len * (len + 1) / 2 - (long long)m * (m + 1) / 2;
    std::swap(cur, next);
  }
  printf("%lld\n", total);
}
