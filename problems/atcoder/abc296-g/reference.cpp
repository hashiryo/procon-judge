// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
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

namespace geo {
template <class K> class IncrementalConvexHull {
 using P= Point<K>;
 struct Lower {
  set<P> S;
  template <class A, class R> void insert(const P& p, const A& ad, const R& rm) {
   if(where(p) >= 0) return;
   S.insert(p);
   vector<P> l, r;
   auto st= S.find(p);
   for(auto it= st; it != S.begin();) {
    if(--it; l.empty()) {
     l.emplace_back(*it);
     continue;
    }
    if(sgn(cross(*it - p, l.back() - p)) > 0) break;
    rm(*it, l.back()), l.emplace_back(*it);
   }
   for(auto it= st; ++it != S.end();) {
    if(r.empty()) {
     r.emplace_back(*it);
     continue;
    }
    if(sgn(cross(r.back() - p, *it - p)) > 0) break;
    rm(r.back(), *it), r.emplace_back(*it);
   }
   if(l.size() > 1) S.erase(next(S.find(l.back())), S.find(p));
   if(l.size()) ad(l.back(), p);
   if(r.size() > 1) S.erase(next(S.find(p)), S.find(r.back()));
   if(r.size()) ad(p, r.back());
   if(l.size() && r.size()) rm(l[0], r[0]);
  }
  int where(const P& p) const {
   auto r= S.lower_bound(p);
   if(r == S.begin()) return S.size() && *r == p ? 0 : -1;
   if(r == S.end()) return -1;
   return sgn(cross(*prev(r) - p, *r - p));
  }
 } L, U;
 size_t m;
 make_long_t<K> a;
public:
 IncrementalConvexHull(): m(0), a(0) {}
 size_t edge_size() const { return m; }
 make_long_t<K> area() const { return a / 2; }
 // for integer
 make_long_t<K> area2() const { return a; }
 // +1: in, 0: on, -1: out
 int where(const P& p) const {
  int l= L.where(p), u= U.where(-p);
  return !l || !u ? 0 : min(l, u);
 }
 void insert(const P& p) {
  auto ad= [&](const P& q, const P& r) { a+= cross(q, r), ++m; };
  auto rm= [&](const P& q, const P& r) { a-= cross(q, r), --m; };
  L.insert(p, ad, rm), U.insert(-p, ad, rm);
 }
};
}

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using namespace geo;
 int N;
 cin >> N;
 IncrementalConvexHull<int> ch;
 for(int i= 0; i < N; ++i) {
  Point<int> p;
  cin >> p;
  ch.insert(p);
 }
 int Q;
 cin >> Q;
 while(Q--) {
  Point<int> p;
  cin >> p;
  int ans= ch.where(p);
  cout << (ans > 0 ? "IN" : ans < 0 ? "OUT" : "ON") << '\n';
 }
 return 0;
}
