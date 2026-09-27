// abc136-d の愚直解。子供を 1 人ずつ、|S| 以上の偶数 T 回だけ実際に動かす。
// |S| 回動けば全員が RL の境目の 2 マスを行き来する周期 2 の動きに入るので、10^100 回 (偶数) 動いたあとと
// T 回動いたあとの位置は同じになる。関数グラフの周期で 10^100 を割らないので、Period とは別の考え方になる。
// O(|S|^2) なので、小さい S でだけ使う。
#include <cstdio>
#include <string>
#include <vector>

int main() {
  static char buf[200005];
  if (scanf("%200000s", buf) != 1) return 1;
  std::string s = buf;
  int n = s.size(), t = n % 2 == 0 ? n : n + 1;
  std::vector<int> cnt(n, 0);
  for (int i = 0; i < n; ++i) {
    int p = i;
    for (int k = 0; k < t; ++k) p += s[p] == 'L' ? -1 : 1;
    ++cnt[p];
  }
  for (int i = 0; i < n; ++i) printf("%d%c", cnt[i], i == n - 1 ? '\n' : ' ');
}
