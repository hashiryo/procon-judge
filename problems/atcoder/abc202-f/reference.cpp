// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
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
// build counterclockwise
template <class K> class Polygon {
 using P= Point<K>;
 make_long_t<K> a2= 0;
protected:
 vector<P> dat;
 void build() {
  if(dat.empty()) return;
  a2= cross(dat.back(), dat[0]);
  for(int i= this->size(); --i;) a2+= cross(dat[i - 1], dat[i]);
  if(a2 < 0) reverse(dat.begin(), dat.end()), a2= -a2;
 }
 template <int opp> inline bool contain(const Segment<K>& s) const {
  assert(dat.size());
  if(where(s.p) == opp || where(s.q) == opp) return false;
  vector<P> ps, qs;
  for(const auto& e: edges())
   if(auto cp= cross_points(s, e); cp.size()) ps.insert(ps.end(), cp.begin(), cp.end());
  if(ps.empty()) return true;
  sort(ps.begin(), ps.end()), ps.erase(unique(ps.begin(), ps.end()), ps.end());
  for(int i= ps.size(); --i;) qs.emplace_back((ps[i] + ps[i - 1]) / 2);
  for(const auto& q: qs)
   if(where(q) == opp) return false;
  return true;
 }
public:
 Polygon() {}
 Polygon(const vector<P>& ps): dat(ps) { build(); }
 inline int prev(int i) const { return i ? i - 1 : (int)this->size() - 1; }
 inline int next(int i) const { return i + 1 >= (int)this->size() ? 0 : i + 1; }
 const P& operator[](int i) const { return dat[i]; }
 auto begin() const { return dat.begin(); }
 auto end() const { return dat.end(); }
 size_t size() const { return dat.size(); }
 vector<Segment<K>> edges() const {
  vector<Segment<K>> es;
  for(int i= 0, e= dat.size(); i < e; ++i) es.emplace_back(dat[i], dat[next(i)]);
  return es;
 }
 // assuming no self-intersections
 bool is_convex() const {
  assert(dat.size());
  for(int i= dat.size(); i--;)
   if(P a= dat[i], b= a - dat[prev(i)], c= dat[next(i)] - a; sgn(cross(b, c)) < 0) return false;
  return true;
 }
 make_long_t<K> area() const { return a2 / 2; }
 // for integer
 make_long_t<K> area2() const { return a2; }
 // 1: in, 0: on, -1: out
 int where(const P& p) const {
  assert(dat.size());
  bool in= false;
  for(int i= dat.size(); i--;) {
   Point a= dat[i] - p, b= dat[next(i)] - p;
   if(a.y > b.y) swap(a, b);
   int s= sgn(cross(a, b));
   if(!s && sgn(dot(a, b)) <= 0) return 0;
   if(s < 0 && sgn(a.y) <= 0 && 0 < sgn(b.y)) in= !in;
  }
  return in ? 1 : -1;
 }
 bool in(const Segment<K>& s) const { return contain<-1>(s); }
 bool out(const Segment<K>& s) const { return contain<+1>(s); }
 friend ostream& operator<<(ostream& os, const Polygon& g) {
  for(int i= 0, e= g.size(); i < e; ++i) os << "--" << g[i] << "-";
  return os;
 }
 friend Visualizer& operator<<(Visualizer& vis, const Polygon& g) {
  vis.ofs << "Polygon" << '\n';
  for(const auto& p: g) vis << p;
  return vis.ofs << "..." << '\n', vis;
 }
};
template <class K> make_long_t<K> dist2(const Polygon<K>& g, const Point<K>& p) {
 if(g.where(p) != -1) return 0;
 make_long_t<K> ret= numeric_limits<make_long_t<K>>::max();
 for(const auto& e: g.edges()) ret= min(ret, dist2(e, p));
 return ret;
}
template <class K> make_long_t<K> dist2(const Point<K>& p, const Polygon<K>& g) { return dist2(g, p); }
template <class K> make_long_t<K> dist2(const Polygon<K>& g, const Line<K>& l) {
 make_long_t<K> ret= numeric_limits<make_long_t<K>>::max();
 for(const auto& e: g.edges()) ret= min(ret, dist2(e, l));
 return ret;
}
template <class K> make_long_t<K> dist2(const Line<K>& l, const Polygon<K>& g) { return dist2(g, l); }
template <class K> make_long_t<K> dist2(const Polygon<K>& g, const Segment<K>& s) {
 if(g.where(s.p) != -1 || g.where(s.q) != -1) return 0;
 make_long_t<K> ret= numeric_limits<make_long_t<K>>::max();
 for(const auto& e: g.edges()) ret= min(ret, dist2(e, s));
 return ret;
}
template <class K> make_long_t<K> dist2(const Segment<K>& s, const Polygon<K>& g) { return dist2(g, s); }
template <class K> make_long_t<K> dist2(const Polygon<K>& g, const Polygon<K>& h) {
 make_long_t<K> ret= numeric_limits<make_long_t<K>>::max();
 for(const auto& e: g.edges()) ret= min(ret, dist2(h, e));
 return ret;
}
template <class K> Polygon<K> Affine<K>::operator()(const Polygon<K>& g) {
 vector<Point<K>> ps;
 for(const auto& p: g) ps.emplace_back((*this)(p));
 return Polygon(ps);
}
}

#include <iostream>
#include <utility>
#include <type_traits>
#include <cassert>
template <class Uint> constexpr inline Uint mod_inv(Uint a, Uint mod) {
 std::make_signed_t<Uint> x= 1, y= 0, z= 0;
 for (Uint q= 0, b= mod, c= 0; b;) z= x, x= y, y= z - y * (q= a / b), c= a, a= b, b= c - b * q;
 return assert(a == 1), x < 0 ? mod - (-x) % mod : x % mod;
}
namespace math_internal {
using namespace std;
using u8= unsigned char;
using u32= unsigned;
using i64= long long;
using u64= unsigned long long;
using u128= __uint128_t;
struct MP_Na {  // mod < 2^32
 u32 mod;
 constexpr MP_Na(): mod(0) {}
 constexpr MP_Na(u32 m): mod(m) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return u64(l) * r % mod; }
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 norm(u32 n) const { return n; }
 constexpr inline u32 plus(u64 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 diff(u64 l, u32 r) const { return l-= r, l >> 63 ? l + mod : l; }
};
template <class u_t, class du_t, u8 B> struct MP_Mo {  // mod < 2^32, mod < 2^62
 u_t mod;
 constexpr MP_Mo(): mod(0), iv(0), r2(0) {}
 constexpr MP_Mo(u_t m): mod(m), iv(inv(m)), r2(-du_t(mod) % mod) {}
 constexpr inline u_t mul(u_t l, u_t r) const { return reduce(du_t(l) * r); }
 constexpr inline u_t set(u_t n) const { return mul(n, r2); }
 constexpr inline u_t get(u_t n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u_t norm(u_t n) const { return n >= mod ? n - mod : n; }
 constexpr inline u_t plus(u_t l, u_t r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u_t diff(u_t l, u_t r) const { return l-= r, l >> (B - 1) ? l + (mod << 1) : l; }
private:
 u_t iv, r2;
 static constexpr u_t inv(u_t n, int e= 6, u_t x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u_t reduce(const du_t &w) const { return u_t(w >> B) + mod - ((du_t(u_t(w) * iv) * mod) >> B); }
};
using MP_Mo32= MP_Mo<u32, u64, 32>;
using MP_Mo64= MP_Mo<u64, u128, 64>;
struct MP_Br {  // 2^20 < mod <= 2^41
 u64 mod;
 constexpr MP_Br(): mod(0), x(0) {}
 constexpr MP_Br(u64 m): mod(m), x((u128(1) << 84) / m) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem(u128(l) * r); }
 static constexpr inline u64 set(u64 n) { return n; }
 constexpr inline u64 get(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 norm(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 plus(u64 l, u64 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u64 diff(u64 l, u64 r) const { return l-= r, l >> 63 ? l + (mod << 1) : l; }
private:
 u64 x;
 constexpr inline u128 quo(const u128 &n) const { return (n * x) >> 84; }
 constexpr inline u64 rem(const u128 &n) const { return n - quo(n) * mod; }
};
template <class du_t, u8 B> struct MP_D2B1 {  // mod < 2^63, mod < 2^64
 u64 mod;
 constexpr MP_D2B1(): mod(0), s(0), d(0), v(0) {}
 constexpr MP_D2B1(u64 m): mod(m), s(__builtin_clzll(m)), d(m << s), v(u128(-1) / d) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem((u128(l) * r) << s) >> s; }
 constexpr inline u64 set(u64 n) const { return n; }
 constexpr inline u64 get(u64 n) const { return n; }
 constexpr inline u64 norm(u64 n) const { return n; }
 constexpr inline u64 plus(du_t l, u64 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u64 diff(du_t l, u64 r) const { return l-= r, l >> B ? l + mod : l; }
private:
 u8 s;
 u64 d, v;
 constexpr inline u64 rem(const u128 &u) const {
  u128 q= (u >> 64) * v + u;
  u64 r= u64(u) - (q >> 64) * d - d;
  if (r > u64(q)) r+= d;
  if (r >= d) r-= d;
  return r;
 }
};
using MP_D2B1_1= MP_D2B1<u64, 63>;
using MP_D2B1_2= MP_D2B1<u128, 127>;
template <class u_t, class MP> constexpr u_t pow(u_t x, u64 k, const MP &md) {
 for (u_t ret= md.set(1);; x= md.mul(x, x))
  if (k & 1 ? ret= md.mul(ret, x) : 0; !(k>>= 1)) return ret;
}
}
#include <type_traits>
namespace math_internal {
struct m_b {};
struct s_b: m_b {};
}
template <class mod_t> constexpr bool is_modint_v= std::is_base_of_v<math_internal::m_b, mod_t>;
template <class mod_t> constexpr bool is_staticmodint_v= std::is_base_of_v<math_internal::s_b, mod_t>;
namespace math_internal {
template <class MP, u64 MOD> struct SB: s_b {
protected:
 static constexpr MP md= MP(MOD);
};
template <class U, class B> struct MInt: public B {
 using Uint= U;
 static constexpr inline auto mod() { return B::md.mod; }
 constexpr MInt(): x(0) {}
 template <class T, typename= enable_if_t<is_modint_v<T> && !is_same_v<T, MInt>>> constexpr MInt(T v): x(B::md.set(v.val() % B::md.mod)) {}
 constexpr MInt(__int128_t n): x(B::md.set((n < 0 ? ((n= (-n) % B::md.mod) ? B::md.mod - n : n) : n % B::md.mod))) {}
 constexpr MInt operator-() const { return MInt() - *this; }
#define FUNC(name, op) \
 constexpr MInt name const { \
  MInt ret; \
  return ret.x= op, ret; \
 }
 FUNC(operator+(const MInt & r), B::md.plus(x, r.x))
 FUNC(operator-(const MInt & r), B::md.diff(x, r.x))
 FUNC(operator*(const MInt & r), B::md.mul(x, r.x))
 FUNC(pow(u64 k), math_internal::pow(x, k, B::md))
#undef FUNC
 constexpr MInt operator/(const MInt& r) const { return *this * r.inv(); }
 constexpr MInt& operator+=(const MInt& r) { return *this= *this + r; }
 constexpr MInt& operator-=(const MInt& r) { return *this= *this - r; }
 constexpr MInt& operator*=(const MInt& r) { return *this= *this * r; }
 constexpr MInt& operator/=(const MInt& r) { return *this= *this / r; }
 constexpr bool operator==(const MInt& r) const { return B::md.norm(x) == B::md.norm(r.x); }
 constexpr bool operator!=(const MInt& r) const { return !(*this == r); }
 constexpr bool operator<(const MInt& r) const { return B::md.norm(x) < B::md.norm(r.x); }
 constexpr inline MInt inv() const { return mod_inv<U>(val(), B::md.mod); }
 constexpr inline Uint val() const { return B::md.get(x); }
 friend ostream& operator<<(ostream& os, const MInt& r) { return os << r.val(); }
 friend istream& operator>>(istream& is, MInt& r) {
  i64 v;
  return is >> v, r= MInt(v), is;
 }
private:
 Uint x;
};
template <u64 MOD> using MP_B= conditional_t < (MOD < (1 << 30)) & MOD, MP_Mo32, conditional_t < MOD < (1ull << 32), MP_Na, conditional_t<(MOD < (1ull << 62)) & MOD, MP_Mo64, conditional_t<MOD<(1ull << 41), MP_Br, conditional_t<MOD<(1ull << 63), MP_D2B1_1, MP_D2B1_2>>>>>;
template <u64 MOD> using ModInt= MInt < conditional_t<MOD<(1 << 30), u32, u64>, SB<MP_B<MOD>, MOD>>;
}
using math_internal::ModInt;

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using Mint= ModInt<int(1e9) + 7>;
 int N;
 cin >> N;
 vector<Point<int>> ps(N);
 for(int i= 0; i < N; ++i) cin >> ps[i];
 sort(ps.begin(), ps.end());
 int parity[N][N][N], inside[N][N][N];
 for(int i= N; i--;)
  for(int j= N; j--;) {
   if(i == j) continue;
   for(int k= N; k--;) {
    if(i == k || j == k) continue;
    Polygon<int> g({ps[i], ps[j], ps[k]});
    parity[i][j][k]= g.area2() & 1, inside[i][j][k]= 0;
    for(int l= N; l--;) {
     if(l == i || l == j || l == k) continue;
     inside[i][j][k]+= g.where(ps[l]) != -1;
    }
   }
  }
 Mint ans= 0, pw[N + 1];
 pw[0]= 1;
 for(int i= 0; i < N; ++i) pw[i + 1]= pw[i] + pw[i];
 Mint dp[N][N][2][2];
 for(int must= N; must--;) {
  for(int i= must; i < N; ++i)
   for(int j= must; j < N; ++j)
    for(int k= 2; k--;) dp[i][j][k][0]= dp[i][j][k][1]= 0;
  for(int i= must + 1; i < N; ++i) dp[must][i][0][0]= dp[must][i][0][1]= 1;
  for(int i= must; i < N; ++i)
   for(int j= i + 1; j < N; ++j)
    for(int k= 2; k--;)
     for(int l= j + 1; l < N; ++l) {
      bool f= ccw(ps[i], ps[j], ps[l]) == CLOCKWISE;
      dp[j][l][k ^ parity[must][j][l]][f]+= dp[i][j][k][f] * pw[inside[must][j][l]];
     }
  for(int j= must + 1; j < N; ++j)
   for(int k= 2; k--;) {
    Mint up= 0, lo= 0;
    for(int i= must; i < j; ++i) up+= dp[i][j][k][0], lo+= dp[i][j][k][1];
    ans+= up * lo;
   }
 }
 cout << ans - N * (N - 1) / 2 << '\n';
 return 0;
}
