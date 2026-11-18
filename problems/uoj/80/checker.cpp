// uoj-80 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 重みの和の最大は 1 つに決まるので、期待出力の 1 つ目の数と同じことを確かめる。組はどれでもよいので、男ごとの相手が
// 0 か女の番号で、女が重ならず、どの組も入力にある組で、組の重みの和が答えの和に等しいことを確かめる。
#include <cstdio>
#include <map>
#include <vector>
using namespace std;
using i64 = long long;
int main(int argc, char **argv) {
  if (argc < 4) return 2;
  FILE *in = fopen(argv[1], "r");
  FILE *out = fopen(argv[2], "r");
  FILE *ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int nl, nr, m;
  if (fscanf(in, "%d %d %d", &nl, &nr, &m) != 3) return 2;
  map<pair<int, int>, i64> w;
  for (int i = 0; i < m; ++i) {
    int v, u;
    i64 c;
    if (fscanf(in, "%d %d %lld", &v, &u, &c) != 3) return 2;
    w[{v, u}] = c;
  }
  i64 want, got;
  if (fscanf(ans, "%lld", &want) != 1) return 2;
  if (fscanf(out, "%lld", &got) != 1) return fprintf(stderr, "和を読めません\n"), 1;
  if (got != want) return fprintf(stderr, "和が %lld ですが、期待出力は %lld です\n", got, want), 1;
  vector<char> used(nr + 1, 0);
  i64 sum = 0;
  for (int v = 1; v <= nl; ++v) {
    int u;
    if (fscanf(out, "%d", &u) != 1) return fprintf(stderr, "男 %d の相手を読めません\n", v), 1;
    if (u == 0) continue;
    if (u < 0 || u > nr || used[u]) return fprintf(stderr, "男 %d の相手 %d が範囲の外か重複です\n", v, u), 1;
    auto it = w.find({v, u});
    if (it == w.end()) return fprintf(stderr, "男 %d と女 %d は組めません\n", v, u), 1;
    used[u] = 1, sum += it->second;
  }
  if (sum != got) return fprintf(stderr, "組の重みの和が %lld で、答えの %lld と違います\n", sum, got), 1;
  return 0;
}
