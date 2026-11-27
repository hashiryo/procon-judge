#include <cstdint>
#include <cstdio>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
using namespace geo;
using ll= long long;
using i128= __int128_t;
static ll sq(ll v) { return v * v; }
static ll distance_sq(const Point<ll>& a, const Point<ll>& b) { return sq(a.x - b.x) + sq(a.y - b.y); }
static int ccw3(const Point<ll>& a, const Point<ll>& b, const Point<ll>& c) { return sgn(cross(b - a, c - a)); }
enum Containment { OUTSIDE= -1, ON= 0, INSIDE= 1 };
static Containment containment_diameter(const Point<ll>& a, const Point<ll>& b, const Point<ll>& p) {
 i128 ux= 2 * (i128)p.x - a.x - b.x;
 i128 uy= 2 * (i128)p.y - a.y - b.y;
 i128 lhs= ux * ux + uy * uy;
 i128 rhs= (i128)distance_sq(a, b);
 if(lhs < rhs) return INSIDE;
 if(lhs == rhs) return ON;
 return OUTSIDE;
}
static i128 incircle_det(const Point<ll>& a, const Point<ll>& b, const Point<ll>& c, const Point<ll>& p) {
 i128 ax= a.x - p.x, ay= a.y - p.y;
 i128 bx= b.x - p.x, by= b.y - p.y;
 i128 cx= c.x - p.x, cy= c.y - p.y;
 i128 A= ax * ax + ay * ay;
 i128 B= bx * bx + by * by;
 i128 C= cx * cx + cy * cy;
 return ax * (by * C - cy * B) - ay * (bx * C - cx * B) + A * (bx * cy - by * cx);
}
static Containment containment_circumcircle(const Point<ll>& a, const Point<ll>& b, const Point<ll>& c, const Point<ll>& p) {
 int s= ccw3(a, b, c);
 if(s == 0) {
  ll dab= distance_sq(a, b), dbc= distance_sq(b, c), dac= distance_sq(a, c);
  if(dab >= dbc && dab >= dac) return containment_diameter(a, b, p);
  if(dbc >= dab && dbc >= dac) return containment_diameter(b, c, p);
  return containment_diameter(a, c, p);
 }
 i128 det= incircle_det(a, b, c, p);
 i128 val= (s > 0) ? det : -det;
 if(val > 0) return INSIDE;
 if(val == 0) return ON;
 return OUTSIDE;
}
struct Support {
 int m= 0;
 Point<ll> a, b, c;
};
static Containment containment_by_support(const Support& S, const Point<ll>& p) {
 if(S.m == 0) return OUTSIDE;
 if(S.m == 1) return (p.x == S.a.x && p.y == S.a.y) ? ON : OUTSIDE;
 if(S.m == 2) return containment_diameter(S.a, S.b, p);
 return containment_circumcircle(S.a, S.b, S.c, p);
}
static void reduce_if_collinear(Support& S) {
 if(S.m != 3) return;
 if(ccw3(S.a, S.b, S.c) != 0) return;
 ll dab= distance_sq(S.a, S.b), dbc= distance_sq(S.b, S.c), dac= distance_sq(S.a, S.c);
 if(dab >= dbc && dab >= dac) S.m= 2;
 else if(dbc >= dab && dbc >= dac) S.m= 2, S.a= S.b, S.b= S.c;
 else S.m= 2, S.b= S.c;
}
static uint64_t xs64(uint64_t& st) { return st^= st << 7, st^= st >> 9; }
static void deterministic_shuffle(vector<Point<ll>>& a, uint64_t seed= 0x9E3779B97F4A7C15ULL) {
 if(a.size() <= 1) return;
 uint64_t s= seed;
 for(int i= (int)a.size() - 1; i >= 1; --i) {
  uint64_t r= xs64(s);
  int j= (int)(r % (uint64_t)(i + 1));
  if(i != j) swap(a[i], a[j]);
 }
}
static Support mec_support(vector<Point<ll>> pts) {
 deterministic_shuffle(pts);
 Support S;
 for(int i= 0; i < (int)pts.size(); ++i) {
  if((int)containment_by_support(S, pts[i]) >= 0) continue;
  S.m= 1, S.a= pts[i];
  for(int j= 0; j < i; ++j) {
   if((int)containment_by_support(S, pts[j]) >= 0) continue;
   S.m= 2, S.a= pts[i], S.b= pts[j];
   for(int k= 0; k < j; ++k) {
    if((int)containment_by_support(S, pts[k]) >= 0) continue;
    S.m= 3, S.a= pts[i], S.b= pts[j], S.c= pts[k];
    reduce_if_collinear(S);
   }
  }
 }
 return S;
}
signed main() {
 int N;
 if(scanf("%d", &N) != 1) return 0;
 vector<Point<ll>> pts(N);
 for(int i= 0; i < N; ++i) {
  long long x, y;
  scanf("%lld %lld", &x, &y);
  pts[i]= Point<ll>(x, y);
 }
 Support S= mec_support(pts);
 vector<char> out(N, '0');
 if(S.m == 1) {
  for(int i= 0; i < N; ++i)
   if(pts[i].x == S.a.x && pts[i].y == S.a.y) out[i]= '1';
 } else if(S.m == 2) {
  for(int i= 0; i < N; ++i)
   if(containment_diameter(S.a, S.b, pts[i]) == ON) out[i]= '1';
 } else {
  for(int i= 0; i < N; ++i)
   if(containment_circumcircle(S.a, S.b, S.c, pts[i]) == ON) out[i]= '1';
 }
 for(int i= 0; i < N; ++i) putchar(out[i]);
 putchar('\n');
 return 0;
}
