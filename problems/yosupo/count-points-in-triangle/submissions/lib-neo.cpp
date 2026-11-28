#include <array>
#include <cstdio>
#include <vector>
#include "neo/algo/compute/geo/count_points_in_triangle.hpp"
using namespace std;
signed main() {
 int N;
 scanf("%d", &N);
 vector<Point<long long>> A(N);
 for(auto& p: A) scanf("%lld %lld", &p.x, &p.y);
 int M;
 scanf("%d", &M);
 vector<Point<long long>> B(M);
 for(auto& p: B) scanf("%lld %lld", &p.x, &p.y);
 int Q;
 scanf("%d", &Q);
 vector<array<int, 3>> qs(Q);
 for(auto& q: qs) scanf("%d %d %d", &q[0], &q[1], &q[2]);
 auto ans= count_points_in_triangle(A, B, qs);
 for(int x: ans) printf("%d\n", x);
 return 0;
}
