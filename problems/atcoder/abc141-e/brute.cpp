// abc141-e の愚直解。始まりの組 l1 < l2 を全部試し、S[l1..] と S[l2..] が先頭から何文字一致するかを
// 1 文字ずつ比べて数える。その組で取れる長さは、一致した長さと l2 - l1 (重ならない条件) の小さいほう。
// ハッシュも接尾辞配列も二分探索も使わないので、提出とは別の考え方になる。O(N^3) なので、短い文字列でだけ使う。
#include <algorithm>
#include <iostream>
#include <string>

int main() {
  int n;
  std::string s;
  if (!(std::cin >> n >> s)) return 1;
  int best = 0;
  for (int l1 = 0; l1 < n; ++l1)
    for (int l2 = l1 + 1; l2 < n; ++l2) {
      int k = 0;
      while (l2 + k < n && s[l1 + k] == s[l2 + k]) ++k;
      best = std::max(best, std::min(k, l2 - l1));
    }
  std::cout << best << '\n';
}
