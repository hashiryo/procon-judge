// abc272-f の愚直解。f(S, i) と f(T, j) を全部文字列として作り、組ごとに std::string の比較で数える。
// ハッシュと二分探索で共通の接頭辞を求めないので、RollingHash とは別の考え方になる。O(N^3) なので、
// 小さい入力でだけ使う。
#include <iostream>
#include <string>
#include <vector>

int main() {
  int n;
  std::string s, t;
  if (!(std::cin >> n >> s >> t)) return 1;
  std::vector<std::string> fs(n), ft(n);
  for (int i = 0; i < n; ++i) fs[i] = s.substr(i) + s.substr(0, i), ft[i] = t.substr(i) + t.substr(0, i);
  long long count = 0;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) count += fs[i] <= ft[j];
  std::cout << count << '\n';
}
