#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/convex_layers.hpp"
using namespace std;
signed main() {
 int N;
 scanf("%d", &N);
 vector<Point<long long>> ps(N);
 for(auto& p: ps) scanf("%lld %lld", &p.x, &p.y);
 auto ans= convex_layers(ps);
 for(int x: ans) printf("%d\n", x);
 return 0;
}
