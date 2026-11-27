#include <cstdint>
#include <iostream>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 int N;
 cin >> N;
 vector<Point<long long>> A(N);
 for(auto& p: A) cin >> p;
 int M;
 cin >> M;
 vector<Point<long long>> B(M);
 for(auto& p: B) cin >> p;
 int W= (M + 63) >> 6;
 vector<uint64_t> left((size_t)N * N * W, 0);
 for(int a= 0; a < N; ++a)
  for(int b= 0; b < N; ++b) {
   if(a == b) continue;
   auto d= A[b] - A[a];
   uint64_t* row= &left[((size_t)a * N + b) * W];
   for(int j= 0; j < M; ++j)
    if(sgn(cross(d, B[j] - A[a])) > 0) row[j >> 6]|= 1ULL << (j & 63);
  }
 int Q;
 cin >> Q;
 while(Q--) {
  int a, b, c;
  cin >> a >> b >> c;
  int ans= 0;
  if(int s= sgn(cross(A[b] - A[a], A[c] - A[a])); s) {
   if(s < 0) swap(b, c);
   const uint64_t* p= &left[((size_t)a * N + b) * W];
   const uint64_t* q= &left[((size_t)b * N + c) * W];
   const uint64_t* r= &left[((size_t)c * N + a) * W];
   for(int w= 0; w < W; ++w) ans+= __builtin_popcountll(p[w] & q[w] & r[w]);
  }
  cout << ans << '\n';
 }
 return 0;
}
