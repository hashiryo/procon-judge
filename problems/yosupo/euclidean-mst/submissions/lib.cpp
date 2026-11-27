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
 vector<Point<long long>> ps(N);
 for(auto& p: ps) cin >> p;
 using L= make_long_t<long long>;
 const L INF= (L)1 << 120;
 vector<L> best(N, INF);
 vector<int> from(N, -1);
 vector<char> used(N, 0);
 best[0]= 0;
 for(int t= 0; t < N; ++t) {
  int v= -1;
  for(int i= 0; i < N; ++i)
   if(!used[i] && (v == -1 || best[i] < best[v])) v= i;
  used[v]= 1;
  if(from[v] != -1) cout << v << ' ' << from[v] << '\n';
  for(int i= 0; i < N; ++i)
   if(!used[i])
    if(L d= dist2(ps[v], ps[i]); d < best[i]) best[i]= d, from[i]= v;
 }
 return 0;
}
