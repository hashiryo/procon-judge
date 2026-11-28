// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-11-28、Library 597d8f64e)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp (縦の切片の合併を積分する別の方法) と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <utility>
#include <vector>
#include <algorithm>
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

namespace geo {
template <class K> struct Line {
 using P= Point<K>;
 P p, d;  // p+td
 Line() {}
 // p + td
 Line(const P& p, const P& d): p(p), d(d) { assert(sgn(norm2(d))); }
 // ax+by+c=0 ................. ax+by+c>0: left, ax+by+c=0: on, ax+by+c<0: right
 Line(K a, K b, K c) {
  int sa= sgn(a), sb= sgn(b);
  assert(sa || sb);
  d= P{b, -a}, p= sb ? P{0, -c / b} : P{-c / a, 0};
 }
 bool operator==(const Line& l) const { return !sgn(cross(d, l.d)) && !where(l.p); }
 bool operator!=(const Line& l) const { return sgn(cross(d, l.d)) || where(l.p); }
 // +1: left, 0: on, -1: right
 int where(const P& q) const { return sgn(cross(d, q - p)); }
 P project(const P& q) const { return p + dot(q - p, d) / norm2(d) * d; }
 // return  a,b,c of ax+by+c=0
 tuple<K, K, K> coef() const { return make_tuple(-d.y, d.x, cross(p, d)); }
 friend ostream& operator<<(ostream& os, const Line& l) { return os << l.p << " + t" << l.d; }
 friend Visualizer& operator<<(Visualizer& vis, const Line& l) {
  auto [a, b, c]= l.coef();
  return vis.ofs << "Line " << a << " " << b << " " << c << "\n", vis;
 }
};
// p + t(q-p)
template <class K> Line<K> line_through(const Point<K>& p, const Point<K>& q) { return Line(p, q - p); }
template <class K> bool is_parallel(const Line<K>& l, const Line<K>& m) { return !sgn(cross(l.d, m.d)); }
template <class K> bool is_orthogonal(const Line<K>& l, const Line<K>& m) { return !sgn(dot(l.d, m.d)); }
// 1 : properly crossing, 0 : disjoint parallel, 2 : same line
template <class K> vector<Point<K>> cross_points(const Line<K>& l, const Line<K>& m) {
 K a= cross(m.d, l.d), b= cross(l.p - m.p, l.d);
 if(sgn(a)) return {m.p + b / a * m.d};  // properly crossing
 if(sgn(b)) return {};                   // disjoint parallel
 return {m.p, m.p + m.d};                // same line
}
// perpendicular bisector ............ p on leftside
template <class K> Line<K> bisector(const Point<K>& p, const Point<K>& q) { return Line((p + q) / 2, !(q - p)); }
// angle bisector ........... parallel -> 1 line, non-parallel -> 2 lines
template <class K> vector<Line<K>> bisector(const Line<K>& l, const Line<K>& m) {
 auto cp= cross_points(l, m);
 if(cp.size() != 1) return {Line((l.p + m.p) / 2, l.d)};
 auto d= l.d / norm(l.d) + m.d / norm(m.d);
 return {Line(cp[0], d), Line(cp[0], !d)};
}
template <class K> make_long_t<K> dist2(const Line<K>& l, const Point<K>& p) {
 make_long_t<K> a= cross(l.d, p - l.p);
 return a * a / norm2(l.d);
}
template <class K> make_long_t<K> dist2(const Point<K>& p, const Line<K>& l) { return dist2(l, p); }
template <class K> make_long_t<K> dist2(const Line<K>& l, const Line<K>& m) { return is_parallel(l, m) ? dist2(l, m.p) : 0; }
template <class K> Affine<K> reflect(const Line<K>& l) {
 K a= l.d.x * l.d.x, b= l.d.x * l.d.y * 2, c= l.d.y * l.d.y, d= a + c;
 a/= d, b/= d, c/= d, d= a - c;
 return {d, b, b, -d, Point<K>{c * 2 * l.p.x - b * l.p.y, a * 2 * l.p.y - b * l.p.x}};
}
template <class K> Line<K> Affine<K>::operator()(const Line<K>& l) { return line_through((*this)(l.p), (*this)(l.p + l.d)); }
}

namespace geo {
template <class K> struct Segment {
 using P= Point<K>;
 P p, q;
 Segment() {}
 Segment(const P& p, const P& q): p(p), q(q) {}
 // do not consider the direction
 bool operator==(const Segment& s) const { return (p == s.p && q == s.q) || (p == s.q && q == s.p); }
 bool operator!=(const Segment& s) const { return !(*this == s); }
 bool on(const P& r) const { return ccw(p, q, r) == ON_SEGMENT; }
 P& operator[](int i) { return i ? q : p; }
 const P& operator[](int i) const { return i ? q : p; }
 long double length() const { return dist(p, q); }
 P closest_point(const P& r) const {
  P d= q - p;
  K a= dot(r - p, d), b;
  return sgn(a) > 0 ? sgn(a - (b= norm2(d))) < 0 ? p + a / b * d : q : p;
 }
 friend ostream& operator<<(ostream& os, const Segment& s) { return os << s.p << "---" << s.q; }
 friend Visualizer& operator<<(Visualizer& vis, const Segment& s) { return vis.ofs << "Segment " << s.p.x << " " << s.p.y << " " << s.q.x << " " << s.q.y << "\n", vis; }
};
// 1: properly crossing, 0: no intersect, 2: same line
template <class K> vector<Point<K>> cross_points(const Segment<K>& s, const Line<K>& l) {
 Point d= s.q - s.p;
 K a= cross(d, l.d), b= cross(l.p - s.p, l.d);
 if(sgn(a)) {
  if(b/= a; sgn(b) < 0 || sgn(b - 1) > 0) return {};  // no intersect
  else return {s.p + b * d};                          // properly crossing}
 }
 if(sgn(b)) return {};  // disjoint parallel
 return {s.p, s.q};     // same line
}
template <class K> vector<Point<K>> cross_points(const Line<K>& l, const Segment<K>& s) { return cross_points(s, l); }
// 2: same line, 0: no intersect, 1: ...
template <class K> vector<Point<K>> cross_points(const Segment<K>& s, const Segment<K>& t) {
 Point d= s.q - s.p, e= t.q - t.p;
 K a= cross(d, e), b= cross(t.p - s.p, e);
 if(sgn(a)) {
  if(b/= a; sgn(b) < 0 || sgn(b - 1) > 0) return {};                       // no intersect
  if(b= cross(d, s.p - t.p) / a; sgn(b) < 0 || sgn(b - 1) > 0) return {};  // no intersect
  return {t.p + b * e};                                                    // properly crossing
 }
 if(sgn(b)) return {};  // disjoint parallel
 vector<Point<K>> ps;   // same line
 auto insert_if_possible= [&](const Point<K>& p) {
  for(auto q: ps)
   if(p == q) return;
  ps.emplace_back(p);
 };
 if(sgn(dot(t.p - s.p, t.q - s.p)) <= 0) insert_if_possible(s.p);
 if(sgn(dot(t.p - s.q, t.q - s.q)) <= 0) insert_if_possible(s.q);
 if(sgn(dot(s.p - t.p, s.q - t.p)) <= 0) insert_if_possible(t.p);
 if(sgn(dot(s.p - t.q, s.q - t.q)) <= 0) insert_if_possible(t.q);
 return ps;
}
enum INTERSECTION { CROSSING, TOUCHING, DISJOINT, OVERLAP };
ostream& operator<<(ostream& os, INTERSECTION i) { return os << (i == CROSSING ? "CROSSING" : i == TOUCHING ? "TOUCHING" : i == DISJOINT ? "DISJOINT" : "OVERLAP"); }
template <class K> INTERSECTION intersection(const Segment<K>& s, const Segment<K>& t) {
 auto cp= cross_points(s, t);
 return cp.size() == 0 ? DISJOINT : cp.size() == 2 ? OVERLAP : cp[0] == s.p || cp[0] == s.q || cp[0] == t.p || cp[0] == t.q ? TOUCHING : CROSSING;
}
template <class K> make_long_t<K> dist2(const Segment<K>& s, const Point<K>& p) { return dist2(p, s.closest_point(p)); }
template <class K> make_long_t<K> dist2(const Point<K>& p, const Segment<K>& s) { return dist2(s, p); }
template <class K> make_long_t<K> dist2(const Segment<K>& s, const Line<K>& l) { return cross_points(s, l).size() ? 0 : min(dist2(s.p, l), dist2(s.q, l)); }
template <class K> make_long_t<K> dist2(const Line<K>& l, const Segment<K>& s) { return dist2(s, l); }
template <class K> make_long_t<K> dist2(const Segment<K>& s, const Segment<K>& t) { return cross_points(s, t).size() ? 0 : min({dist2(s, t.p), dist2(s, t.q), dist2(t, s.p), dist2(t, s.q)}); }
template <class K> Segment<K> Affine<K>::operator()(const Segment<K>& s) { return {(*this)(s.p), (*this)(s.q)}; }
}

namespace geo {
template <class R> struct Circle {
 using P= Point<R>;
 P o;
 R r;
 Circle() {}
 Circle(const P& o, R r): o(o), r(r) {}
 long double area() const { return r * r * M_PI; }
 // +1: in, 0: on, -1: out
 int where(const P& p) const { return sgn(r * r - dist2(p, o)); }
 // +1: intersect, 0: contact, -1: disjoint
 int where(const Line<R>& l) const { return sgn(r * r - dist2(l, o)); }
 // true: c in *this
 bool in(const Circle& c) const {
  R a= c.r - r;
  return sgn(a) <= 0 && sgn(dist2(o, c.o) - a * a) <= 0;
 }
 vector<Line<R>> tangent(const P& p) const {
  P d= p - o, e= !d;
  R b= norm2(d), a= b - r * r;
  if(int s= sgn(a); s < 0) return {};
  else if(s == 0) return {{p, e}};
  d*= r, e*= sqrt(a);
  return {Line(p, !(d + e)), Line(p, !(d - e))};
 }
 friend ostream& operator<<(ostream& os, const Circle& c) { return os << c.o << " " << c.r; }
 friend Visualizer& operator<<(Visualizer& vis, const Circle& c) { return vis.ofs << "Circle " << c.o.x << " " << c.o.y << " " << c.r << '\n', vis; }
};
// 2: properly intersect, 1: contact, 0: disjoint, 3: same
// counter-clockwise of c and clockwise of d
template <class R> vector<Point<R>> cross_points(const Circle<R>& c, const Circle<R>& d) {
 Point v= d.o - c.o;
 R g= norm2(v), a= c.r - d.r, b= c.r + d.r;
 if(!sgn(g)) {
  if(sgn(a)) return {};
  return {{c.o.x + c.r, c.o.y}, {c.o.x - c.r, c.o.y}, {c.o.x, c.o.y + c.r}};
 }
 int in= sgn(g - a * a), out= sgn(g - b * b);
 if(in < 0 || out > 0) return {};
 if(!in) return {(c.r * d.o - d.r * c.o) / a};
 if(!out) return {(c.r * d.o + d.r * c.o) / b};
 R e= (a * b + g) / (g * 2);
 Point q= c.o + e * v, n= !v * sqrt(c.r * c.r / g - e * e);
 return {q - n, q + n};
}
// 2: properly intersect, 1: contact, 0: disjoint
// direction of l
template <class R> vector<Point<R>> cross_points(const Circle<R>& c, const Line<R>& l) {
 Point<R> v= l.p - c.o;
 R a= norm2(l.d), b= dot(l.d, v) / a, d= b * b - (norm2(v) - c.r * c.r) / a;
 int s= sgn(d);
 if(s < 0) return {};
 if(!s) return {l.p - b * l.d};
 d= sqrt(d);
 return {l.p - (b + d) * l.d, l.p - (b - d) * l.d};
}
template <class R> vector<Point<R>> cross_points(const Line<R>& l, const Circle<R>& c) { return cross_points(c, l); }
template <class R> vector<Point<R>> cross_points(const Circle<R>& c, const Segment<R>& s) {
 Point<R> u= s.q - s.p, v= s.p - c.o;
 R a= norm2(u), b= dot(u, v) / a, d= b * b - (norm2(v) - c.r * c.r) / a;
 int t= sgn(d);
 if(t < 0) return {};
 if(!t && sgn(b) <= 0 && sgn(1 + b) >= 0) return {s.p - b * u};
 d= sqrt(d), a= -b - d, b= -b + d;
 vector<Point<R>> ps;
 if(0 <= sgn(a) && sgn(a - 1) <= 0) ps.emplace_back(s.p + a * u);
 if(0 <= sgn(b) && sgn(b - 1) <= 0) ps.emplace_back(s.p + b * u);
 return ps;
}
template <class R> vector<Point<R>> cross_points(const Segment<R>& s, const Circle<R>& c) { return cross_points(c, s); }
template <class R> Circle<R> circumscribed_circle(const Point<R>& A, const Point<R>& B, const Point<R>& C) {
 Point u= !(B - A), v= C - A, o= (A + B + dot(C - B, v) / dot(u, v) * u) / 2;
 return {o, dist(A, o)};
}
template <class R> Circle<R> inscribed_circle(const Point<R>& A, const Point<R>& B, const Point<R>& C) {
 R a= dist(B, C), b= dist(C, A), c= dist(A, B), s= (a + b + c) / 2;
 return {(a * A + b * B + c * C) / (s * 2), sqrt((s - a) * (s - b) * (s - c) / s)};
}
template <class R> vector<Line<R>> common_tangent(const Circle<R>& c, const Circle<R>& d) {
 Point u= d.o - c.o, v= !u;
 R g= norm2(u), b;
 if(!sgn(g)) return {};  // same origin
 vector<Line<R>> ls;
 for(R a: {c.r - d.r, c.r + d.r}) {
  if(int s= sgn(b= g - a * a); !s) ls.emplace_back(Line(c.o + c.r * a / g * u, v));
  else if(s > 0) {
   Point x= a / g * u, y= sqrt(b) / g * v, e= x + y, f= x - y;
   ls.emplace_back(Line(c.o + c.r * e, !e)), ls.emplace_back(Line(c.o + c.r * f, !f));
  }
 }
 return ls;
}
}

using namespace std;
// 各円の周のうち他のどの円の内側にも入らない弧を集め、Green の定理で面積にする。O(N^2 log N)。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using R= long double;
 int N;
 cin >> N;
 vector<Circle<R>> cs(N);
 for(auto& c: cs) cin >> c.o >> c.r;
 const R tau= 2 * numbers::pi_v<R>;
 sort(cs.begin(), cs.end(), [](const Circle<R>& a, const Circle<R>& b) {
  if(a.o.x != b.o.x) return a.o.x < b.o.x;
  if(a.o.y != b.o.y) return a.o.y < b.o.y;
  return a.r < b.r;
 });
 cs.erase(unique(cs.begin(), cs.end(), [](const Circle<R>& a, const Circle<R>& b) { return a.o == b.o && a.r == b.r; }), cs.end());
 auto arc_area= [](const Circle<R>& c, R a, R b) {
  return (c.r * c.r * (b - a) + c.r * (c.o.x * (sin(b) - sin(a)) - c.o.y * (cos(b) - cos(a)))) / 2;
 };
 R ans= 0;
 for(int i= 0; i < (int)cs.size(); ++i) {
  const R ri= cs[i].r;
  vector<pair<R, R>> iv;
  bool covered= false;
  for(int j= 0; j < (int)cs.size(); ++j) {
   if(i == j) continue;
   const R rj= cs[j].r, d= dist(cs[i].o, cs[j].o);
   if(sgn(d + ri - rj) <= 0) {
    covered= true;
    break;
   }
   if(sgn(d + rj - ri) <= 0 || sgn(d - (ri + rj)) >= 0) continue;
   R c= (ri * ri + d * d - rj * rj) / (2 * ri * d);
   c= clamp(c, R(-1), R(1));
   R a= atan2(cs[j].o.y - cs[i].o.y, cs[j].o.x - cs[i].o.x) - acos(c);
   R b= a + 2 * acos(c);
   a= fmod(a, tau), a+= a < 0 ? tau : 0;
   b= fmod(b, tau), b+= b < 0 ? tau : 0;
   if(a <= b) iv.emplace_back(a, b);
   else iv.emplace_back(a, tau), iv.emplace_back(0, b);
  }
  if(covered || iv.empty()) {
   if(!covered) ans+= numbers::pi_v<R> * ri * ri;
   continue;
  }
  sort(iv.begin(), iv.end());
  R prev= 0;
  for(auto [a, b]: iv) {
   if(b <= prev) continue;
   if(a > prev) ans+= arc_area(cs[i], prev, a);
   prev= max(prev, b);
  }
  if(prev < tau) ans+= arc_area(cs[i], prev, tau);
 }
 cout << fixed << setprecision(10) << ans << '\n';
 return 0;
}
