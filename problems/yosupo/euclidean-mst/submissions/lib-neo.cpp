#include <algorithm>
#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/euclidean_mst.hpp"
using namespace std;
signed main() {
 int N;
 scanf("%d", &N);
 vector<Point<long long>> ps(N);
 for(auto& p: ps) scanf("%lld %lld", &p.x, &p.y);
 auto es= euclidean_mst(ps);
 for(auto& [u, v]: es)
  if(u > v) swap(u, v);
 sort(es.begin(), es.end());
 for(auto [u, v]: es) printf("%d %d\n", u, v);
 return 0;
}
