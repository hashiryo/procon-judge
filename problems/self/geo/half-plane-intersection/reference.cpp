// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-11-28、Library 597d8f64e)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <cmath>
#include <iomanip>
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

namespace geo {
template <class K> struct Convex: Polygon<K> {
 using P= Point<K>;
 Convex() {}
 Convex(vector<P> ps, bool strict= true) {
  sort(ps.begin(), ps.end()), ps.erase(unique(ps.begin(), ps.end()), ps.end());
  int n= ps.size(), k= 0;
  auto& ch= this->dat;
  if(n < 3) {
   ch= ps;
   return;
  }
  ch.resize(2 * n);
  for(int i= 0; i < n; ch[k++]= ps[i++])
   while(k > 1 && sgn(cross(ch[k - 1] - ch[k - 2], ps[i] - ch[k - 2])) < strict) --k;
  for(int i= n - 1, t= k; i--; ch[k++]= ps[i])
   while(k > t && sgn(cross(ch[k - 1] - ch[k - 2], ps[i] - ch[k - 2])) < strict) --k;
  ch.resize(k - 1), this->build();
 }
 pair<P, P> farthest_pair() const {
  auto& ch= this->dat;
  int n= ch.size(), i= 0, j= 0;
  if(n == 2) return {ch[0], ch[1]};
  if(n == 1) return {ch[0], ch[0]};
  for(int k= n; k--;) {
   if(ch[i] > ch[k]) i= k;
   if(ch[j] < ch[k]) j= k;
  }
  pair<P, P> ret{ch[i], ch[j]};
  make_long_t<K> mx= dist2(ch[i], ch[j]);
  for(int si= i, sj= j; i != sj || j != si;) {
   if(int ni= this->next(i), nj= this->next(j); sgn(cross(ch[ni] - ch[i], ch[nj] - ch[j])) < 0) i= ni;
   else j= nj;
   if(make_long_t<K> len= dist2(ch[i], ch[j]); mx < len) mx= len, ret= {ch[i], ch[j]};
  }
  return ret;
 }
 long double diameter() const {
  auto [p, q]= farthest_pair();
  return dist(p, q);
 }
 // side>0 => left, side<0 => right
 Convex cut(const Line<K>& l, int side= 1) const {
  Convex ret;
  for(const auto& e: this->edges()) {
   auto d= e.q - e.p;
   K a= cross(d, l.d), b= cross(l.p - e.p, l.d);
   int s= sgn(b);
   if(s * side >= 0) ret.dat.emplace_back(e.p);
   if(s && sgn(a))
    if(b/= a; 0 < sgn(b) && sgn(b - 1) < 0) ret.dat.emplace_back(e.p + b * d);
  }
  return ret.build(), ret;
 }
 // { (x,y): (x,y) in polygon and (ax+by+c) * side >= 0 }
 Convex cut(K a, K b, K c, int side= 1) const {
  int sa= sgn(a), sb= sgn(b), sc= sgn(c);
  if(!sa && !sb) return sc * side < 0 ? Convex() : *this;
  return cut(Line<K>(a, b, c), side);
 }
 friend Affine<K>;
};
template <class K> pair<Point<K>, Point<K>> farthest_pair(const vector<Point<K>>& ps) { return Convex<K>(ps).farthest_pair(); }
template <class K> Convex<K> Affine<K>::operator()(const Convex<K>& c) {
 Convex<K> d;
 for(const auto& p: c) d.dat.emplace_back((*this)(p));
 return d.build(), d;
}
}

using namespace std;
// 箱から始めて、半平面を 1 本ずつで切る。O(N * 多角形の頂点数)。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using R= long double;
 using P= Point<R>;
 int N;
 cin >> N;
 constexpr R B= 200000;
 Convex<R> c({P(-B, -B), P(B, -B), P(B, B), P(-B, B)});
 for(int i= 0; i < N; ++i) {
  R a, b, d;
  cin >> a >> b >> d;
  c= c.cut(Line<R>(a, b, d), 1);
 }
 cout << fixed << setprecision(10) << c.area() << '\n';
 return 0;
}
