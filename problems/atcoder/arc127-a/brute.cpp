// arc127-a の愚直解。x を 1 から N まで 1 ずつ増やし、10 進の桁を配列に持ったまま、先頭に並ぶ 1 を数えて足す。
// 桁ごとの状態を持つ Automaton の DP を使わないので、提出とは別の考え方になる。O(N) なので、
// 小さい N でだけ使う。
#include <cstdio>

int main() {
  long long n;
  if (scanf("%lld", &n) != 1) return 1;
  int digit[20] = {};  // digit[0] が一の位。x = 0 から始める
  int len = 1;
  long long total = 0;
  for (long long x = 1; x <= n; ++x) {
    int i = 0;  // x - 1 に 1 を足す
    while (digit[i] == 9) digit[i++] = 0;
    ++digit[i];
    if (i + 1 > len) len = i + 1;
    for (int j = len - 1; j >= 0 && digit[j] == 1; --j) ++total;
  }
  printf("%lld\n", total);
}
