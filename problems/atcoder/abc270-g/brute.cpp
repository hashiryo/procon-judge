// abc270-g の愚直解。X_0 = S から X_i を順に求め、最初に G になった i を答える。列は P 通りの値しか
// 取らないので、X_0 から X_{P-1} までに G が無ければ、その先にも無い。一次関数の合成を小さな歩幅と
// 大きな歩幅に分けて探す (Baby-step Giant-step) ことはしないので、提出とは別の考え方になる。
// O(P) なので、P の小さい入力でだけ使う。
#include <cstdio>

int main() {
  int t;
  if (scanf("%d", &t) != 1) return 1;
  while (t--) {
    long long p, a, b, s, g;
    if (scanf("%lld %lld %lld %lld %lld", &p, &a, &b, &s, &g) != 5) return 1;
    long long x = s, answer = -1;
    for (long long i = 0; i < p; ++i) {
      if (x == g) {
        answer = i;
        break;
      }
      x = (a * x + b) % p;
    }
    printf("%lld\n", answer);
  }
}
