// 期待出力を作る参照実装。submissions/lib-seg2d.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/abc266/tasks/abc266_h
#include <iostream>
#include <vector>
#include <vector>
#include <algorithm>
#include <numeric>
#include <map>
#include <set>
#include <cassert>
#include <tuple>
#include <array>
#include <type_traits>
#include <cstddef>
template <class T> static constexpr bool tuple_like_v= false;
template <class... Args> static constexpr bool tuple_like_v<std::tuple<Args...>> = true;
template <class T, class U> static constexpr bool tuple_like_v<std::pair<T, U>> = true;
template <class T, size_t K> static constexpr bool tuple_like_v<std::array<T, K>> = true;
template <class T> auto to_tuple(const T &t) {
 if constexpr (tuple_like_v<T>) return std::apply([](auto &&...x) { return std::make_tuple(x...); }, t);
}
template <class T> auto forward_tuple(const T &t) {
 if constexpr (tuple_like_v<T>) return std::apply([](auto &&...x) { return std::forward_as_tuple(x...); }, t);
}
template <class T> static constexpr bool array_like_v= false;
template <class T, size_t K> static constexpr bool array_like_v<std::array<T, K>> = true;
template <class T, class U> static constexpr bool array_like_v<std::pair<T, U>> = std::is_convertible_v<T, U>;
template <class T> static constexpr bool array_like_v<std::tuple<T>> = true;
template <class T, class U, class... Args> static constexpr bool array_like_v<std::tuple<T, U, Args...>> = array_like_v<std::tuple<T, Args...>> && std::is_convertible_v<U, T>;
template <class T> auto to_array(const T &t) {
 if constexpr (array_like_v<T>) return std::apply([](auto &&...x) { return std::array{x...}; }, t);
}
template <class T> using to_tuple_t= decltype(to_tuple(T()));
template <class T> using to_array_t= decltype(to_array(T()));
template <class pos_t, class M> class SegmentTree_2D {
 using T= typename M::T;
 using Pos= std::array<pos_t, 2>;
 std::vector<pos_t> xs;
 std::vector<Pos> yxs;
 std::vector<int> id, tol;
 std::vector<T> val;
 template <class P> using canbe_Pos= std::is_convertible<to_tuple_t<P>, std::tuple<pos_t, pos_t>>;
 template <class P> using canbe_PosV= std::is_convertible<to_tuple_t<P>, std::tuple<pos_t, pos_t, T>>;
 template <class P, class U> static constexpr bool canbe_Pos_and_T_v= std::conjunction_v<canbe_Pos<P>, std::is_convertible<U, T>>;
 int sz;
 inline int x2i(pos_t x) const { return std::lower_bound(xs.begin(), xs.end(), x) - xs.begin(); }
 inline int y2i(pos_t y) const {
  return std::lower_bound(yxs.begin(), yxs.end(), Pos{y, 0}, [](const Pos& a, const Pos& b) { return a[0] < b[0]; }) - yxs.begin();
 }
 inline int xy2i(pos_t x, pos_t y) const {
  Pos p{y, x};
  auto it= std::lower_bound(yxs.begin(), yxs.end(), p);
  return assert(p == *it), it - yxs.begin();
 }
 template <bool z, size_t k, class P> inline auto get_(const P& p) {
  if constexpr(z) return std::get<k>(p);
  else return std::get<k>(p.first);
 }
 template <bool z, class XYW> inline void build(const XYW* xyw, int n, const T& v= M::ti()) {
  xs.resize(n);
  for(int i= n; i--;) xs[i]= get_<z, 0>(xyw[i]);
  std::sort(xs.begin(), xs.end()), xs.erase(std::unique(xs.begin(), xs.end()), xs.end()), id.resize((sz= 1 << (32 - __builtin_clz(xs.size()))) + xs.size() + 1);
  std::vector<int> ord(n);
  for(int j= n; j--;)
   for(int i= x2i(get_<z, 0>(xyw[j])) + sz; i; i>>= 1) ++id[i + 1];
  for(int i= 1, e= sz + xs.size(); i < e; ++i) id[i + 1]+= id[i];
  val.assign(id.back() * 2, M::ti()), tol.resize(id[sz] + 1), std::iota(ord.begin(), ord.end(), 0), std::sort(ord.begin(), ord.end(), [&](int i, int j) { return get_<z, 1>(xyw[i]) == get_<z, 1>(xyw[j]) ? get_<z, 0>(xyw[i]) < get_<z, 0>(xyw[j]) : get_<z, 1>(xyw[i]) < get_<z, 1>(xyw[j]); });
  {
   std::vector<int> ptr= id;
   for(int r: ord)
    for(int i= x2i(get_<z, 0>(xyw[r])) + sz, j= -1; i; j= i, i>>= 1) {
     int p= ptr[i]++;
     if constexpr(z) {
      if constexpr(std::tuple_size_v<XYW> == 3) val[id[i + 1] + p]= std::get<2>(xyw[r]);
      else val[id[i + 1] + p]= v;
     } else val[id[i + 1] + p]= xyw[r].second;
     if(j != -1) tol[p + 1]= !(j & 1);
    }
   for(int i= 1, e= id[sz]; i < e; ++i) tol[i + 1]+= tol[i];
   for(int i= 0, e= sz + xs.size(); i < e; ++i) {
    auto dat= val.begin() + id[i] * 2;
    for(int j= id[i + 1] - id[i]; --j > 0;) dat[j]= M::op(dat[j * 2], dat[j * 2 + 1]);
   }
  }
  yxs.resize(n);
  for(int i= n; i--;) yxs[i]= {get_<z, 1>(xyw[ord[i]]), get_<z, 0>(xyw[ord[i]])};
 }
 inline T prdi(int i, int a, int b) const {
  int n= id[i + 1] - id[i];
  T ret= M::ti();
  auto dat= val.begin() + id[i] * 2;
  for(a+= n, b+= n; a < b; a>>= 1, b>>= 1) {
   if(a & 1) ret= M::op(ret, dat[a++]);
   if(b & 1) ret= M::op(dat[--b], ret);
  }
  return ret;
 }
 template <bool z> inline void seti(int i, int j, T v) {
  auto dat= val.begin() + id[i] * 2;
  j+= id[i + 1] - id[i];
  if constexpr(z) dat[j]= v;
  else dat[j]= M::op(dat[j], v);
  for(; j;) j>>= 1, dat[j]= M::op(dat[2 * j], dat[2 * j + 1]);
 }
 template <bool z> inline void set_(pos_t x, pos_t y, T v) {
  for(int i= 1, p= xy2i(x, y);;) {
   if(seti<z>(i, p - id[i], v); i >= sz) break;
   if(int lc= tol[p] - tol[id[i]], rc= (p - id[i]) - lc; tol[p + 1] - tol[p]) p= id[2 * i] + lc, i= 2 * i;
   else p= id[2 * i + 1] + rc, i= 2 * i + 1;
  }
 }
public:
 template <class P, typename= std::enable_if_t<std::disjunction_v<canbe_Pos<P>, canbe_PosV<P>>>> SegmentTree_2D(const P* p, size_t n) { build<1>(p, n); }
 template <class P, typename= std::enable_if_t<std::disjunction_v<canbe_Pos<P>, canbe_PosV<P>>>> SegmentTree_2D(const std::vector<P>& p): SegmentTree_2D(p.data(), p.size()) {}
 template <class P, typename= std::enable_if_t<canbe_Pos<P>::value>> SegmentTree_2D(const std::set<P>& p): SegmentTree_2D(std::vector(p.begin(), p.end())) {}
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const P* p, size_t n, const U& v) { build<1>(p, n, v); }
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const std::vector<P>& p, const U& v): SegmentTree_2D(p.data(), p.size(), v) {}
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const std::set<P>& p, const U& v): SegmentTree_2D(std::vector(p.begin(), p.end()), v) {}
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const std::pair<P, U>* p, size_t n) { build<0>(p, n); }
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const std::vector<std::pair<P, U>>& p): SegmentTree_2D(p.data(), p.size()) {}
 template <class P, class U, typename= std::enable_if_t<canbe_Pos_and_T_v<P, U>>> SegmentTree_2D(const std::map<P, U>& p): SegmentTree_2D(std::vector(p.begin(), p.end())) {}
 // [l,r) x [u,d)
 T prod(pos_t l, pos_t r, pos_t u, pos_t d) const {
  T ret= M::ti();
  int L= x2i(l), R= x2i(r);
  auto dfs= [&](auto& dfs, int i, int a, int b, int c, int d) -> void {
   if(c == d || R <= a || b <= L) return;
   if(L <= a && b <= R) return ret= M::op(ret, prdi(i, c, d)), void();
   int m= (a + b) / 2, ac= tol[id[i] + c] - tol[id[i]], bc= c - ac, ad= tol[id[i] + d] - tol[id[i]], bd= d - ad;
   dfs(dfs, i * 2, a, m, ac, ad), dfs(dfs, i * 2 + 1, m, b, bc, bd);
  };
  return dfs(dfs, 1, 0, sz, y2i(u), y2i(d)), ret;
 }
 void set(pos_t x, pos_t y, T v) { set_<1>(x, y, v); }
 void mul(pos_t x, pos_t y, T v) { set_<0>(x, y, v); }
 T get(pos_t x, pos_t y) const { return val[xy2i(x, y) + id[2]]; }
};

using namespace std;
struct RMQ {
 using T= long long;
 static T ti() { return -(1ll << 60); }
 static T op(T l, T r) { return max(l, r); }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<array<int, 4>> dat(N);
 for(int i= 0; i < N; ++i) cin >> dat[i][0] >> dat[i][1] >> dat[i][2] >> dat[i][3];
 sort(dat.begin(), dat.end(), [&](auto& l, auto& r) { return l[2] == r[2] ? l[0] < r[0] : l[2] < r[2]; });
 set<array<long long, 2>> st1, st2;
 st1.insert({0, 0}), st2.insert({0, 0});
 for(int i= 0; i <= N; ++i) {
  auto [t, x, y, a]= dat[i];
  st1.insert({x, x + y - t}), st2.insert({x, x - y + t});
 }
 long long ans= 0;
 SegmentTree_2D<long long, RMQ> seg1(st1), seg2(st2);
 static constexpr long long INF= 1ll << 60;
 seg1.set(0, 0, 0), seg2.set(0, 0, 0);
 for(int i= 0; i < N; ++i) {
  auto [t, x, y, a]= dat[i];
  auto dp= max(seg1.prod(-INF, x + 1, x + y - t, INF), seg2.prod(x, INF, -INF, x - y + t + 1));
  if(dp < 0) continue;
  dp+= a;
  ans= max(ans, dp);
  seg1.mul(x, x + y - t, dp), seg2.mul(x, x - y + t, dp);
 }
 cout << ans << '\n';
 return 0;
}
