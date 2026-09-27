// abc213-f の愚直解。lcp[i][j] = (S[i] == S[j] ? lcp[i+1][j+1] + 1 : 0) を i の大きい方から 1 行ずつ埋め、
// 行 k の和をそのまま答えにする。接尾辞配列も接尾辞木も作らないので、提出とは別の考え方になる。
// O(N^2) なので小さい入力でだけ使う。
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<char> buf(n + 1);
  if (scanf("%s", buf.data()) != 1) return 1;
  std::string s(buf.data(), n);
  // next は行 i + 1、cur は行 i。列 n は空の接尾辞なので常に 0。
  std::vector<int> next(n + 1, 0), cur(n + 1, 0);
  std::vector<long long> ans(n);
  for (int i = n - 1; i >= 0; --i) {
    long long sum = 0;
    for (int j = 0; j < n; ++j) {
      cur[j] = s[i] == s[j] ? next[j + 1] + 1 : 0;
      sum += cur[j];
    }
    cur[n] = 0;
    ans[i] = sum;
    std::swap(cur, next);
  }
  for (int i = 0; i < n; ++i) printf("%lld\n", ans[i]);
}
