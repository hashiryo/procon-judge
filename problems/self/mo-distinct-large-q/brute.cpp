// 愚直解。クエリごとに区間を走査して、値の集合の大きさを出す。seed 1000 以上の小さい入力だけを想定する。
#include <cstdio>
#include <set>
#include <vector>
using namespace std;

int main() {
  int n, q;
  if (scanf("%d %d", &n, &q) != 2) return 0;
  vector<int> a(n);
  for (auto& x : a) scanf("%d", &x);
  while (q--) {
    int l, r;
    scanf("%d %d", &l, &r);
    set<int> s;
    for (int i = l - 1; i < r; ++i) s.insert(a[i]);
    printf("%d\n", (int)s.size());
  }
  return 0;
}
