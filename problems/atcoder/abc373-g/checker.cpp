// atcoder-abc373-g のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 期待出力が -1 なら提出も -1 であること、そうでなければ R が 1 から N の順列で、どの 2 本の線分 P_i Q_{R_i} も交わらない
// (端を共有する、触れる、重なるも交わりとみなす) ことを、整数の外積で確かめる。順列はどれでもよいので、期待出力からは
// -1 かどうかだけを読む。
#include <algorithm>
#include <cstdio>
#include <vector>
using namespace std;
using i64 = long long;
struct Pt { i64 x, y; };
i64 cross(Pt o, Pt a, Pt b) { return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x); }
bool on_seg(Pt a, Pt b, Pt p) { return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x) && min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y); }
bool meet(Pt a, Pt b, Pt c, Pt d) {
  i64 d1 = cross(c, d, a), d2 = cross(c, d, b), d3 = cross(a, b, c), d4 = cross(a, b, d);
  if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) && ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) return true;
  return (d1 == 0 && on_seg(c, d, a)) || (d2 == 0 && on_seg(c, d, b)) || (d3 == 0 && on_seg(a, b, c)) || (d4 == 0 && on_seg(a, b, d));
}
int main(int argc, char** argv) {
  if (argc < 4) return 2;
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "r");
  FILE* ans = fopen(argv[3], "r");
  if (!in || !out || !ans) return 2;
  int n;
  if (fscanf(in, "%d", &n) != 1) return 2;
  vector<Pt> p(n), q(n);
  for (auto& v : p) if (fscanf(in, "%lld %lld", &v.x, &v.y) != 2) return 2;
  for (auto& v : q) if (fscanf(in, "%lld %lld", &v.x, &v.y) != 2) return 2;
  long long e;
  if (fscanf(ans, "%lld", &e) != 1) return 2;
  long long first;
  if (fscanf(out, "%lld", &first) != 1) return fprintf(stderr, "出力を読めません\n"), 1;
  if (e == -1 || first == -1) {
    if (e != first) return fprintf(stderr, "-1 かどうかが期待出力と違います\n"), 1;
  } else {
    vector<int> r(n);
    r[0] = (int)first;
    for (int i = 1; i < n; ++i)
      if (fscanf(out, "%d", &r[i]) != 1) return fprintf(stderr, "R_%d を読めません\n", i + 1), 1;
    vector<int> seen(n + 1, 0);
    for (int i = 0; i < n; ++i) {
      if (r[i] < 1 || r[i] > n || seen[r[i]]++) return fprintf(stderr, "R が順列ではありません (R_%d = %d)\n", i + 1, r[i]), 1;
      --r[i];
    }
    for (int i = 0; i < n; ++i)
      for (int j = i + 1; j < n; ++j)
        if (meet(p[i], q[r[i]], p[j], q[r[j]])) return fprintf(stderr, "線分 %d と %d が交わります\n", i + 1, j + 1), 1;
  }
  char extra[8];
  if (fscanf(out, "%7s", extra) == 1) return fprintf(stderr, "余計な出力があります\n"), 1;
  return 0;
}
