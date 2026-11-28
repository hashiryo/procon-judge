// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-11-28、Library 597d8f64e)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい N では brute.cpp (部分集合をすべて試す) と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cassert>
// clang-format off
template<class T>struct make_long{using type= T;};
template<>struct make_long<char>{using type= short;};
template<>struct make_long<unsigned char>{using type= unsigned short;};
template<>struct make_long<short>{using type= int;};
template<>struct make_long<unsigned short>{using type= unsigned;};
template<>struct make_long<int>{using type= long long;};
template<>struct make_long<unsigned>{using type= unsigned long long;};
template<>struct make_long<long long>{using type= __int128_t;};
template<>struct make_long<unsigned long long>{using type= __uint128_t;};
template<>struct make_long<float>{using type= double;};
template<>struct make_long<double>{using type= long double;};
template<class T> using make_long_t= typename make_long<T>::type;
// clang-format on
namespace geo {
using namespace std;
struct Visualizer {
 ofstream ofs;
 Visualizer(string s= "visualize.txt"): ofs(s) { ofs << fixed << setprecision(10); }
 friend Visualizer& operator<<(Visualizer& vis, const string& s) { return vis.ofs << s, vis; }
};
template <class K> int sgn(K x) {
 if constexpr(is_floating_point_v<K>) {
  static constexpr K EPS= 1e-9;
  return x < -EPS ? -1 : x > EPS;
 } else return x < 0 ? -1 : x > 0;
}
template <class K> K err_floor(K x) {
 K y= floor(x);
 if constexpr(is_floating_point_v<K>)
  if(K z= y + 1, w= x - z; 0 <= sgn(w) && sgn(w - 1) < 0) return z;
 return y;
}
template <class K> K err_ceil(K x) {
 K y= ceil(x);
 if constexpr(is_floating_point_v<K>)
  if(K z= y - 1, w= x - z; 0 < sgn(w + 1) && sgn(w) <= 0) return z;
 return y;
}
template <class K> struct Point {
 K x, y;
 Point(K x= K(), K y= K()): x(x), y(y) {}
 Point& operator+=(const Point& p) { return x+= p.x, y+= p.y, *this; }
 Point& operator-=(const Point& p) { return x-= p.x, y-= p.y, *this; }
 Point& operator*=(K a) { return x*= a, y*= a, *this; }
 Point& operator/=(K a) { return x/= a, y/= a, *this; }
 Point operator+(const Point& p) const { return {x + p.x, y + p.y}; }
 Point operator-(const Point& p) const { return {x - p.x, y - p.y}; }
 Point operator*(K a) const { return {x * a, y * a}; }
 Point operator/(K a) const { return {x / a, y / a}; }
 friend Point operator*(K a, const Point& p) { return {a * p.x, a * p.y}; }
 Point operator-() const { return {-x, -y}; }
 bool operator<(const Point& p) const {
  int s= sgn(x - p.x);
  return s ? s < 0 : sgn(y - p.y) < 0;
 }
 bool operator>(const Point& p) const { return p < *this; }
 bool operator<=(const Point& p) const { return !(p < *this); }
 bool operator>=(const Point& p) const { return !(*this < p); }
 bool operator==(const Point& p) const { return !sgn(x - p.x) && !sgn(y - p.y); }
 bool operator!=(const Point& p) const { return sgn(x - p.x) || sgn(y - p.y); }
 Point operator!() const { return {-y, x}; }  // rotate 90 degree
 friend istream& operator>>(istream& is, Point& p) { return is >> p.x >> p.y; }
 friend ostream& operator<<(ostream& os, const Point& p) { return os << "(" << p.x << ", " << p.y << ")"; }
 friend Visualizer& operator<<(Visualizer& vis, const Point& p) { return vis.ofs << p.x << " " << p.y << "\n", vis; }
};
template <class K> make_long_t<K> dot(const Point<K>& p, const Point<K>& q) { return make_long_t<K>(p.x) * q.x + make_long_t<K>(p.y) * q.y; }
// left turn: > 0, right turn: < 0
template <class K> make_long_t<K> cross(const Point<K>& p, const Point<K>& q) { return make_long_t<K>(p.x) * q.y - make_long_t<K>(p.y) * q.x; }
template <class K> make_long_t<K> norm2(const Point<K>& p) { return dot(p, p); }
template <class K> long double norm(const Point<K>& p) { return sqrt(norm2(p)); }
template <class K> make_long_t<K> dist2(const Point<K>& p, const Point<K>& q) { return norm2(p - q); }
template <class T, class U> long double dist(const T& a, const U& b) { return sqrt(dist2(a, b)); }
enum CCW { COUNTER_CLOCKWISE, CLOCKWISE, ONLINE_BACK, ONLINE_FRONT, ON_SEGMENT };
ostream& operator<<(ostream& os, CCW c) { return os << (c == COUNTER_CLOCKWISE ? "COUNTER_CLOCKWISE" : c == CLOCKWISE ? "CLOCKWISE" : c == ONLINE_BACK ? "ONLINE_BACK" : c == ONLINE_FRONT ? "ONLINE_FRONT" : "ON_SEGMENT"); }
template <class K> CCW ccw(const Point<K>& p0, const Point<K>& p1, const Point<K>& p2) {
 Point a= p1 - p0, b= p2 - p0;
 int s;
 if constexpr(is_floating_point_v<K>) s= sgn(sgn(cross(a, b) / sqrt(norm2(a) * norm2(b))));
 else s= sgn(cross(a, b));
 if(s) return s > 0 ? COUNTER_CLOCKWISE : CLOCKWISE;
 if(K d= dot(a, b); sgn(d) < 0) return ONLINE_BACK;
 else return sgn(d - norm2(a)) > 0 ? ONLINE_FRONT : ON_SEGMENT;
}
template <class K> struct Line;
template <class K> struct Segment;
template <class K> class Polygon;
template <class K> struct Convex;
template <class K> struct Affine {
 K a00= 1, a01= 0, a10= 0, a11= 1;
 Point<K> b;
 Point<K> operator()(const Point<K>& p) const { return {a00 * p.x + a01 * p.y + b.x, a10 * p.x + a11 * p.y + b.y}; }
 Line<K> operator()(const Line<K>& l);
 Segment<K> operator()(const Segment<K>& s);
 Polygon<K> operator()(const Polygon<K>& p);
 Convex<K> operator()(const Convex<K>& c);
 Affine operator*(const Affine& r) const { return {a00 * r.a00 + a01 * r.a10, a00 * r.a01 + a01 * r.a11, a10 * r.a00 + a11 * r.a10, a10 * r.a01 + a11 * r.a11, (*this)(r)}; }
 Affine& operator*=(const Affine& r) { return *this= *this * r; }
};
template <class K> Affine<K> translate(const Point<K>& p) { return {1, 0, 0, 1, p}; }
}

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
