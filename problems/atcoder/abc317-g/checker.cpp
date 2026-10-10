// atcoder-abc317-g のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の Yes / No が期待出力と一致し、Yes なら N 行 M 列の数が、各行は元の行の並べ替えで、各列は 1 から N を 1 つずつ
// 含むことを確かめる。並べ方はどれでもよいので、期待出力からは Yes か No かだけを読む。
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace std;
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n, m;
  if (fscanf(in, "%d %d", &n, &m) != 2) return 2;
  vector<vector<int>> a(n, vector<int>(m));
  for (auto& row : a)
    for (int& x : row) if (fscanf(in, "%d", &x) != 1) return 2;
  char exp[8], got[8];
  if (fscanf(ans, "%7s", exp) != 1) return 2;
  if (fscanf(out, "%7s", got) != 1) return fprintf(stderr, "Yes か No を読めません\n"), 1;
  if (strcmp(got, "Yes") != 0 && strcmp(got, "No") != 0) return fprintf(stderr, "1 行目が %s です\n", got), 1;
  if (strcmp(got, exp) != 0) return fprintf(stderr, "%s と答えましたが、正しくは %s です\n", got, exp), 1;
  if (strcmp(got, "Yes") == 0) {
    vector<vector<int>> b(n, vector<int>(m));
    for (int i = 0; i < n; ++i)
      for (int j = 0; j < m; ++j)
        if (fscanf(out, "%d", &b[i][j]) != 1) return fprintf(stderr, "%d 行 %d 列を読めません\n", i + 1, j + 1), 1;
    for (int i = 0; i < n; ++i) {
      vector<int> x = a[i], y = b[i];
      sort(x.begin(), x.end()), sort(y.begin(), y.end());
      if (x != y) return fprintf(stderr, "%d 行目が元の行の並べ替えではありません\n", i + 1), 1;
    }
    for (int j = 0; j < m; ++j) {
      vector<int> seen(n + 1, 0);
      for (int i = 0; i < n; ++i)
        if (b[i][j] < 1 || b[i][j] > n || seen[b[i][j]]++) return fprintf(stderr, "%d 列目に同じ数があります\n", j + 1), 1;
    }
  }
  char extra[8];
  if (fscanf(out, "%7s", extra) == 1) return fprintf(stderr, "余計な出力があります\n"), 1;
  return 0;
}
