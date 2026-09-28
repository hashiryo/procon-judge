// abc155-e の愚直解。払う額 P を N から 10^(L+2) - 1 まで全部試し (L は N の桁数)、
// P の桁和とおつり P - N の桁和の和の最小を出す。最適な P は 10^(L+1) より小さいので、これで足りる。
// 桁ごとに繰り上がりを状態にしたオートマトンの DP をしないので、提出とは別の考え方になる。
// N が 5 桁までのときだけ使う。
#include <cstdio>

using u64 = unsigned long long;

int digit_sum(u64 x) {
  int s = 0;
  for (; x; x /= 10) s += x % 10;
  return s;
}

int main() {
  char buf[64];
  if (scanf("%63s", buf) != 1) return 1;
  u64 n = 0, top = 100;
  for (char* p = buf; *p; ++p) n = n * 10 + (*p - '0'), top *= 10;
  int best = 1 << 30;
  for (u64 pay = n; pay < top; ++pay) {
    int c = digit_sum(pay) + digit_sum(pay - n);
    if (c < best) best = c;
  }
  printf("%d\n", best);
}
