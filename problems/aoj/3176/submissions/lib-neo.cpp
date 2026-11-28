#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/SegmentArrangement.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, M;
 cin >> N >> M;
 vector<Point<int>> ps(N);
 for(auto& p: ps) cin >> p;
 vector<Segment<int>> ss(M);
 for(int i= 0; i < M; ++i) {
  int u, v;
  cin >> u >> v;
  ss[i]= Segment<int>(ps[--u], ps[--v]);
 }
 SegmentArrangement<int> sa(ss);
 long long ans= 0;
 for(int f= sa.face_size(); f--;) ans= max(ans, (long long)sa.area2(f));
 cout << ans << '\n';
 return 0;
}
