#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/manhattan_mst.hpp"
using namespace std;
signed main() {
 int n;
 scanf("%d", &n);
 vector<Point<long long>> ps(n);
 for(auto& p: ps) scanf("%lld %lld", &p.x, &p.y);
 auto res= manhattan_mst(ps);
 printf("%lld\n", (long long)res.weight);
 for(auto [u, v]: res.edges) printf("%d %d\n", u, v);
 return 0;
}
