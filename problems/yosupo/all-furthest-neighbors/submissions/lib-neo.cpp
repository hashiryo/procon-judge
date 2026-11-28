#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/all_furthest_neighbors.hpp"
using namespace std;
signed main() {
 int T;
 scanf("%d", &T);
 while(T--) {
  int n;
  scanf("%d", &n);
  vector<Point<long long>> p(n);
  for(auto& q: p) scanf("%lld %lld", &q.x, &q.y);
  auto ans= all_furthest_neighbors(p);
  for(int i= 0; i < n; ++i) printf(i ? " %d" : "%d", ans[i]);
  puts("");
 }
 return 0;
}
