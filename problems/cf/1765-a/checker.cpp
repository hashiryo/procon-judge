// cf-1765-a のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 組の数 k は答えが 1 つに決まるので、期待出力の 1 つ目の数と同じことを確かめる。組と水準の付け方はどれでもよいので、
// 文書の組が 1 以上 k 以下、水準がどれも 1 以上 10^9 以下で、どの開発者とどの文書の組でも、見られるか (開発者のその組の水準
// が文書の水準以上か) が入力の表と一致することを確かめる。
#include <cstdio>
#include <string>
#include <vector>
using namespace std;
using i64 = long long;
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  vector<string> a(n);
  static char buf[1024];
  for (auto& s : a) {
    if (fscanf(in, "%1023s", buf) != 1) return 2;
    s = buf;
  }
  i64 want, k;
  if (fscanf(ans, "%lld", &want) != 1) return 2;
  if (fscanf(out, "%lld", &k) != 1) return fprintf(stderr, "k を読めません\n"), 1;
  if (k != want) return fprintf(stderr, "k が %lld ですが、期待出力は %lld です\n", k, want), 1;
  vector<i64> grp(m), lev(m);
  for (int j = 0; j < m; ++j) {
    if (fscanf(out, "%lld", &grp[j]) != 1) return fprintf(stderr, "文書 %d の組を読めません\n", j + 1), 1;
    if (grp[j] < 1 || grp[j] > k) return fprintf(stderr, "文書 %d の組 %lld が範囲の外です\n", j + 1, grp[j]), 1;
  }
  for (int j = 0; j < m; ++j) {
    if (fscanf(out, "%lld", &lev[j]) != 1) return fprintf(stderr, "文書 %d の水準を読めません\n", j + 1), 1;
    if (lev[j] < 1 || lev[j] > 1000000000) return fprintf(stderr, "文書 %d の水準 %lld が範囲の外です\n", j + 1, lev[j]), 1;
  }
  vector<i64> v(k);
  for (int i = 0; i < n; ++i) {
    for (int g = 0; g < k; ++g) {
      if (fscanf(out, "%lld", &v[g]) != 1) return fprintf(stderr, "開発者 %d の水準を読めません\n", i + 1), 1;
      if (v[g] < 1 || v[g] > 1000000000) return fprintf(stderr, "開発者 %d の水準 %lld が範囲の外です\n", i + 1, v[g]), 1;
    }
    for (int j = 0; j < m; ++j) {
      const bool can = v[grp[j] - 1] >= lev[j];
      if (can != (a[i][j] == '1'))
        return fprintf(stderr, "開発者 %d が文書 %d を%s\n", i + 1, j + 1, can ? "見られてしまいます" : "見られません"), 1;
    }
  }
  char extra[8];
  if (fscanf(out, "%7s", extra) == 1) return fprintf(stderr, "余計な出力があります\n"), 1;
  return 0;
}
