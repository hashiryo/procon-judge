// arc060-d の愚直解。w の部分文字列ごとに、長さの真の約数 d を全部試して「長さ d の文字列のくり返しか」を
// 直接確かめ、よい文字列かを決める。そのうえで、先頭 i 文字の分け方の (要素数の最小, その分け方の数) を、
// 最後の要素を全部試す DP で求める。答えが 2 以下になることも、接頭辞の周期の表も使わないので、提出とは別の考え方になる。
// O(|w|^3) くらいなので、|w| が 400 までのときだけ使う。
#include <cstdio>
#include <string>
#include <vector>

int main() {
  char buf[1024];
  if (scanf("%1023s", buf) != 1) return 1;
  std::string w = buf;
  int n = w.size();
  // good[l][r]: w[l..r) がよい文字列か
  std::vector<std::vector<char>> good(n + 1, std::vector<char>(n + 1, 0));
  for (int l = 0; l < n; ++l)
    for (int r = l + 1; r <= n; ++r) {
      int len = r - l;
      bool is_good = true;
      for (int d = 1; d < len && is_good; ++d) {
        if (len % d) continue;
        bool rep = true;
        for (int j = l; j + d < r && rep; ++j) rep = w[j] == w[j + d];
        if (rep) is_good = false;
      }
      good[l][r] = is_good;
    }
  const long long MOD = 1000000007;
  const int INF = 1 << 30;
  std::vector<int> pieces(n + 1, INF);
  std::vector<long long> ways(n + 1, 0);
  pieces[0] = 0, ways[0] = 1;
  for (int r = 1; r <= n; ++r)
    for (int l = 0; l < r; ++l) {
      if (!good[l][r] || pieces[l] == INF) continue;
      if (pieces[l] + 1 < pieces[r]) pieces[r] = pieces[l] + 1, ways[r] = ways[l];
      else if (pieces[l] + 1 == pieces[r]) ways[r] = (ways[r] + ways[l]) % MOD;
    }
  printf("%d\n%lld\n", pieces[n], ways[n]);
}
