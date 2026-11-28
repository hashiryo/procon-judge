#include <iostream>
#include <set>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
// スコアの総和は、3 点以上で一直線でない部分集合の数に等しい (S を凸包の頂点、残りを内側の点と読む)。
// 2^N - 1 - N - C(N,2) から、一直線に並ぶ 3 点以上の部分集合を引く。直線は A x + B y + C = 0 の既約な組で表す。
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
 constexpr long long MOD= 998244353;
 int N;
 cin >> N;
 vector<Point<K>> ps(N);
 for(auto& p: ps) cin >> p;
 vector<long long> pw(N + 1);
 pw[0]= 1;
 for(int i= 0; i < N; ++i) pw[i + 1]= pw[i] * 2 % MOD;
 long long ans= (pw[N] - 1 - N - (long long)N * (N - 1) / 2) % MOD;
 set<Key> seen;
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < i; ++j) {
   i128 a= (i128)ps[j].y - ps[i].y, b= (i128)ps[i].x - ps[j].x;
   i128 c= (i128)ps[j].x * ps[i].y - (i128)ps[i].x * ps[j].y;
   u128 g= gcd128(gcd128(a < 0 ? -a : a, b < 0 ? -b : b), c < 0 ? -c : c);
   if(g > 1) a/= (i128)g, b/= (i128)g, c/= (i128)g;
   if(a < 0 || (a == 0 && b < 0)) a= -a, b= -b, c= -c;
   if(!seen.insert({a, b, c}).second) continue;
   int cnt= 0;
   for(int l= 0; l < N; ++l)
    if(!sgn(cross(ps[j] - ps[i], ps[l] - ps[i]))) ++cnt;
   if(cnt >= 3) ans= (ans - (pw[cnt] - 1 - cnt - (long long)cnt * (cnt - 1) / 2)) % MOD;
  }
 ans%= MOD;
 if(ans < 0) ans+= MOD;
 cout << ans << '\n';
 return 0;
}
