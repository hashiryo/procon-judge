// atcoder-typical90-by のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の Yes / No が期待出力と一致し、Yes なら N 個の向きがどれも 1 以上 8 以下で、時刻 T の位置が B の N 点と
// 1 つずつ一致することを確かめる。向きの組はどれでもよいので、期待出力からは Yes か No かだけを読む。
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <utility>
#include <vector>
using namespace std;
using i64 = long long;
constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n;
  i64 t;
  if (fscanf(in, "%d %lld", &n, &t) != 2) return 2;
  vector<pair<i64, i64>> a(n), b(n);
  for (auto& [x, y] : a) if (fscanf(in, "%lld %lld", &x, &y) != 2) return 2;
  for (auto& [x, y] : b) if (fscanf(in, "%lld %lld", &x, &y) != 2) return 2;
  char exp[8], got[8];
  if (fscanf(ans, "%7s", exp) != 1) return 2;
  if (fscanf(out, "%7s", got) != 1) return fprintf(stderr, "Yes か No を読めません\n"), 1;
  if (strcmp(got, "Yes") != 0 && strcmp(got, "No") != 0) return fprintf(stderr, "1 行目が %s です\n", got), 1;
  if (strcmp(got, exp) != 0) return fprintf(stderr, "%s と答えましたが、正しくは %s です\n", got, exp), 1;
  if (strcmp(got, "Yes") == 0) {
    vector<pair<i64, i64>> p(n);
    for (int i = 0; i < n; ++i) {
      int d;
      if (fscanf(out, "%d", &d) != 1) return fprintf(stderr, "%d 個目の向きを読めません\n", i + 1), 1;
      if (d < 1 || d > 8) return fprintf(stderr, "%d 個目の向き %d が範囲の外です\n", i + 1, d), 1;
      p[i] = {a[i].first + t * DX[d - 1], a[i].second + t * DY[d - 1]};
    }
    sort(p.begin(), p.end()), sort(b.begin(), b.end());
    if (p != b) return fprintf(stderr, "時刻 T の位置が報告の点と一致しません\n"), 1;
  }
  char extra[8];
  if (fscanf(out, "%7s", extra) == 1) return fprintf(stderr, "余計な出力があります\n"), 1;
  return 0;
}
