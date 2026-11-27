#include <algorithm>
#include <iostream>
#include <numeric>
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
 vector<int> ord(N);
 iota(ord.begin(), ord.end(), 0);
 sort(ord.begin(), ord.end(), [&](int i, int j) { return ps[i].y != ps[j].y ? ps[i].y < ps[j].y : ps[i].x < ps[j].x; });
 auto cross3= [&](int a, int b, int c) { return cross(ps[b] - ps[a], ps[c] - ps[a]); };
 vector<int> layer(N, 0);
 int cnt= 0;
 for(int l= 1; cnt < N; ++l) {
  vector<int> h;
  for(int idx: ord) {
   if(layer[idx] && layer[idx] != l) continue;
   while(h.size() >= 2 && cross3(h[h.size() - 2], h.back(), idx) > 0) h.pop_back();
   h.push_back(idx);
  }
  for(int idx: h)
   if(!layer[idx]) ++cnt, layer[idx]= l;
  h.clear();
  for(int i= N; i--;) {
   int idx= ord[i];
   if(layer[idx] && layer[idx] != l) continue;
   while(h.size() >= 2 && cross3(h[h.size() - 2], h.back(), idx) > 0) h.pop_back();
   h.push_back(idx);
  }
  for(int idx: h)
   if(!layer[idx]) ++cnt, layer[idx]= l;
 }
 for(int i= 0; i < N; ++i) cout << layer[i] << '\n';
 return 0;
}
