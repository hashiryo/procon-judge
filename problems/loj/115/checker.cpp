// loj-115 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の YES と NO が期待出力と一致し、YES なら m 個の流量が辺ごとの下限と上限の間にあり、どの頂点でも入る量と出る量が
// 等しいことを確かめる。流れはどれでもよいので、期待出力からは YES か NO だけを読む。
#include <cstdio>
#include <cstring>
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
  vector<int> u(m), v(m);
  vector<i64> lo(m), hi(m);
  for (int i = 0; i < m; ++i)
    if (fscanf(in, "%d %d %lld %lld", &u[i], &v[i], &lo[i], &hi[i]) != 4) return 2;
  char want[8], got[8];
  if (fscanf(ans, "%7s", want) != 1) return 2;
  if (fscanf(out, "%7s", got) != 1) return fprintf(stderr, "YES か NO を読めません\n"), 1;
  if (strcmp(got, want) != 0) return fprintf(stderr, "%s ですが、期待出力は %s です\n", got, want), 1;
  if (strcmp(want, "YES") == 0) {
    vector<i64> bal(n + 1, 0);
    for (int i = 0; i < m; ++i) {
      i64 f;
      if (fscanf(out, "%lld", &f) != 1) return fprintf(stderr, "%d 本目の辺の流量を読めません\n", i + 1), 1;
      if (f < lo[i] || f > hi[i])
        return fprintf(stderr, "%d 本目の辺の流量 %lld が [%lld, %lld] の外です\n", i + 1, f, lo[i], hi[i]), 1;
      bal[u[i]] -= f, bal[v[i]] += f;
    }
    for (int x = 1; x <= n; ++x)
      if (bal[x] != 0) return fprintf(stderr, "頂点 %d で入る量と出る量が %lld だけ違います\n", x, bal[x]), 1;
  }
  char extra[8];
  if (fscanf(out, "%7s", extra) == 1) return fprintf(stderr, "余計な出力があります\n"), 1;
  return 0;
}
