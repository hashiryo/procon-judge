#include <iostream>
#include <set>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
// 直線を A x + B y + C = 0 の既約な組で表し、K 個以上の点を通るものを数える。
using i128= __int128;
using u128= unsigned __int128;
static u128 gcd128(u128 a, u128 b) {
 while(b) {
  u128 t= a % b;
  a= b, b= t;
 }
 return a;
}
struct Key {
 i128 a, b, c;
 bool operator<(const Key& o) const { return a != o.a ? a < o.a : (b != o.b ? b < o.b : c < o.c); }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using K= long long;
 int N, k;
 cin >> N >> k;
 vector<Point<K>> ps(N);
 for(auto& p: ps) cin >> p;
 if(k == 1) {
  cout << "Infinity" << '\n';
  return 0;
 }
 set<Key> lines;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j) {
   int cnt= 0;
   for(int l= 0; l < N; ++l)
    if(!sgn(cross(ps[j] - ps[i], ps[l] - ps[i]))) ++cnt;
   if(cnt < k) continue;
   i128 a= (i128)ps[j].y - ps[i].y, b= (i128)ps[i].x - ps[j].x;
   i128 c= (i128)ps[j].x * ps[i].y - (i128)ps[i].x * ps[j].y;
   u128 g= gcd128(gcd128(a < 0 ? -a : a, b < 0 ? -b : b), c < 0 ? -c : c);
   if(g > 1) a/= (i128)g, b/= (i128)g, c/= (i128)g;
   if(a < 0 || (a == 0 && b < 0)) a= -a, b= -b, c= -c;
   lines.insert({a, b, c});
  }
 cout << (long long)lines.size() << '\n';
 return 0;
}
