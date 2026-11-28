#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/min_enclosing_circle.hpp"
using namespace std;
signed main() {
 int N;
 if(scanf("%d", &N) != 1) return 0;
 vector<Point<long long>> ps(N);
 for(auto& p: ps) scanf("%lld %lld", &p.x, &p.y);
 auto s= min_enclosing_circle(ps);
 for(int i= 0; i < N; ++i) putchar(s.side(ps[i]) == 0 ? '1' : '0');
 putchar('\n');
 return 0;
}
