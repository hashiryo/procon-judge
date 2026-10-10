// 愚直解。N ≤ 7 の入力で、順列を辞書順に全部試し、どの 2 本の線分も交わらない最初のものを出す。交わりは整数の外積で判定する。
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
int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  vector<Pt> p(n), q(n);
  for (auto& v : p) if (scanf("%lld %lld", &v.x, &v.y) != 2) return 1;
  for (auto& v : q) if (scanf("%lld %lld", &v.x, &v.y) != 2) return 1;
  vector<int> r(n);
  for (int i = 0; i < n; ++i) r[i] = i;
  do {
    bool ok = true;
    for (int i = 0; i < n && ok; ++i)
      for (int j = i + 1; j < n && ok; ++j)
        if (meet(p[i], q[r[i]], p[j], q[r[j]])) ok = false;
    if (ok) {
      for (int i = 0; i < n; ++i) printf("%d%c", r[i] + 1, i + 1 == n ? '\n' : ' ');
      return 0;
    }
  } while (next_permutation(r.begin(), r.end()));
  puts("-1");
}
