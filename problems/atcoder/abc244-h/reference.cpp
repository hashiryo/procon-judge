// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/abc244/tasks/abc244_h
#include <iostream>
#include <limits>
#include <algorithm>
#include <set>
#include <array>
#include <cassert>
enum MinMaxEnum { MAXIMIZE= -1, MINIMIZE= 1 };
template <typename T, MinMaxEnum obj= MINIMIZE> class ConvexHullTrick {
 struct Line {
  T k, m;
  mutable T p;
  bool operator<(const Line& o) const { return k < o.k; }
  bool operator<(T x) const { return p < x; }
 };
 static constexpr T INF= std::numeric_limits<T>::max();
 static T lc_div(T a, T b) {
  if constexpr(std::is_integral_v<T>) return a / b - ((a ^ b) < 0 && a % b);
  else return a / b;
 }
 using ms= std::multiset<Line, std::less<>>;
 ms ls;
 bool insect(typename ms::iterator x, typename ms::iterator y) {
  if(y == ls.end()) return x->p= INF, false;
  if(x->k == y->k) x->p= (x->m > y->m ? INF : -INF);
  else x->p= lc_div(y->m - x->m, x->k - y->k);
  return x->p >= y->p;
 }
public:
 void insert(T k, T m) {
  if constexpr(obj == MINIMIZE) k= -k, m= -m;
  auto z= ls.insert({k, m, 0}), y= z++, x= y;
  while(insect(y, z)) z= ls.erase(z);
  if(x != ls.begin() && insect(--x, y)) insect(x, y= ls.erase(y));
  while((y= x) != ls.begin() && (--x)->p >= y->p) insect(x, ls.erase(y));
 }
 bool empty() const { return ls.empty(); }
 std::array<T, 2> query_line(T x) const {
  assert(!ls.empty());
  auto l= ls.lower_bound(x);
  if constexpr(obj == MINIMIZE) return {-l->k, -l->m};
  else return {l->k, l->m};
 }
 T query(T x) const {
  auto [k, m]= query_line(x);
  return k * x + m;
 }
};
template <typename T> class ConvexHullTrick_XY {
 ConvexHullTrick<long double, MINIMIZE> cht_mn;
 ConvexHullTrick<long double, MAXIMIZE> cht_mx;
 T amx= std::numeric_limits<T>::lowest(), amn= std::numeric_limits<T>::max();
public:
 void insert(T a, T b) { cht_mn.insert(a, b), cht_mx.insert(a, b), amn= std::min(amn, a), amx= std::max(amx, a); }
 bool empty() const { return cht_mn.empty(); }
 T get_max(T x, T y) const {
  assert(!cht_mn.empty());
  if(y == 0) return std::max(amn * x, amx * x);
  auto z= (long double)x / y;
  auto [a, b]= y > 0 ? cht_mx.query_line(z) : cht_mn.query_line(z);
  return T(a) * x + T(b) * y;
 }
 T get_min(T x, T y) const {
  assert(!cht_mn.empty());
  if(y == 0) return std::min(amn * x, amx * x);
  auto z= (long double)x / y;
  auto [a, b]= y > 0 ? cht_mn.query_line(z) : cht_mx.query_line(z);
  return T(a) * x + T(b) * y;
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int Q;
 cin >> Q;
 ConvexHullTrick_XY<int64_t> cht;
 while(Q--) {
  int X, Y, A, B;
  cin >> X >> Y >> A >> B;
  cht.insert(X, Y);
  cout << cht.get_max(A, B) << '\n';
 }
 return 0;
}
