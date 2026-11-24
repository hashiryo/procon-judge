// dmoj-dmopc19c3p6 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 辺素な全域木が取れるかどうかは 1 つに決まるので、期待出力が -1 なら提出も -1、そうでなければ提出が -1 でないことを確かめる。
// 解は何でもよいので、提出の文字列が長さ M で 0 から 3 だけから成り、1、2、3 の辺がそれぞれ N 頂点の全域木 (N - 1 本で、閉路が
// 無い) になっていることを確かめる。
#include <cstdio>
#include <numeric>
#include <string>
#include <vector>
using namespace std;
int main(int argc, char **argv) {
  if (argc < 4) return 2;
  FILE *in = fopen(argv[1], "r");
  FILE *out = fopen(argv[2], "r");
  FILE *ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  vector<int> u(m), v(m);
  for (int i = 0; i < m; ++i)
    if (fscanf(in, "%d %d", &u[i], &v[i]) != 2) return 2;
  char want[4000], got[4000];
  if (fscanf(ans, "%3999s", want) != 1) return 2;
  if (fscanf(out, "%3999s", got) != 1) return fprintf(stderr, "出力を読めません\n"), 1;
  const string w = want, g = got;
  if (w == "-1") return g == "-1" ? 0 : (fprintf(stderr, "取れないのに解を出しています\n"), 1);
  if (g == "-1") return fprintf(stderr, "取れるのに -1 です\n"), 1;
  if ((int)g.size() != m) return fprintf(stderr, "長さが %d で、M = %d と違います\n", (int)g.size(), m), 1;
  vector<vector<int>> par(4, vector<int>(n + 1));
  for (auto &p : par) iota(p.begin(), p.end(), 0);
  auto find = [&](int t, int x) {
    while (par[t][x] != x) x = par[t][x] = par[t][par[t][x]];
    return x;
  };
  int cnt[4] = {};
  for (int i = 0; i < m; ++i) {
    if (g[i] < '0' || g[i] > '3') return fprintf(stderr, "%d 文字目が 0 から 3 でありません\n", i + 1), 1;
    const int t = g[i] - '0';
    if (t == 0) continue;
    const int a = find(t, u[i]), b = find(t, v[i]);
    if (a == b) return fprintf(stderr, "E_%d に閉路があります (辺 %d)\n", t, i + 1), 1;
    par[t][a] = b, ++cnt[t];
  }
  for (int t = 1; t <= 3; ++t)
    if (cnt[t] != n - 1) return fprintf(stderr, "E_%d の辺が %d 本で、N - 1 = %d 本ではありません\n", t, cnt[t], n - 1), 1;
  return 0;
}
