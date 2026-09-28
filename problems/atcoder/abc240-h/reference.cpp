// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/abc240/tasks/abc240_h

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <string>
#include <vector>
#include <algorithm>
template <class String> struct SuffixArray {
 String s;
 std::vector<int> sa;
 static inline std::vector<int> sa_is(const std::vector<int> &s, int K) {
  const int n= s.size();
  std::vector<char> t(n);
  std::vector<int> bkt(K, 0), bkt_l(K), bkt_r(K), sa(n), p1;
  t.back()= true;
  for (int i= n; --i;)
   if (t[i - 1]= (s[i - 1] < s[i] || (t[i] && s[i - 1] == s[i])); t[i] && !t[i - 1]) p1.push_back(i);
  std::reverse(p1.begin(), p1.end());
  const int n1= p1.size();
  for (int i= n; i--;) ++bkt[s[i]];
  for (int i= 0, sum= 0; i < K; ++i) sum+= bkt[i], bkt_r[i]= sum, bkt_l[i]= sum - bkt[i];
  std::vector<int> s1(n1), sa1(n1);
  std::fill_n(sa.begin(), n, -1), std::copy_n(bkt_r.begin(), K, bkt.begin());
  for (int i= n1; i--;) sa[--bkt[s[p1[i]]]]= p1[i];
  std::copy_n(bkt_l.begin(), K, bkt.begin());
  for (int i= 0, j; i < n; ++i)
   if ((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt[s[j]]++]= j;
  std::copy_n(bkt_r.begin(), K, bkt.begin());
  for (int i= n, j; i--;)
   if ((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt[s[j]]]= j;
  for (int i= 0, j= 0; i < n; ++i)
   if (t[sa[i]] && sa[i] > 0 && !t[sa[i] - 1]) sa1[j++]= sa[i];
  int name= 0;
  for (int i= 0, prev= -1, j, pos; i < n1; ++i, sa[pos]= name - 1)
   for (j= 0, pos= sa1[i];; ++j)
    if (prev == -1 || s[pos + j] != s[prev + j] || t[pos + j] != t[prev + j]) {
     prev= pos, ++name;
     break;
    } else if (j && ((t[pos + j] && !t[pos + j - 1]) || (t[prev + j] && !t[prev + j - 1]))) break;
  for (int i= n1; i--;) s1[i]= sa[p1[i]];
  if (name != n1) sa1= sa_is(s1, name);
  else
   for (int i= n1; i--;) sa1[s1[i]]= i;
  std::copy_n(bkt_r.begin(), K, bkt.begin()), std::fill_n(sa.begin(), n, -1);
  for (int i= n1; i--;) sa[--bkt[s[p1[sa1[i]]]]]= p1[sa1[i]];
  for (int i= 0, j; i < n; ++i)
   if ((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt_l[s[j]]++]= j;
  for (int i= n, j; i--;)
   if ((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt_r[s[j]]]= j;
  return sa;
 }
public:
 SuffixArray(const String &S): s(S) {
  std::vector<int> s_cpy(s.size() + 1);
  if constexpr (std::is_convertible_v<String, std::string>) std::copy(s.begin(), s.end(), s_cpy.begin()), sa= sa_is(s_cpy, 128), sa.erase(sa.begin());
  else {
   auto v= s;
   sort(v.begin(), v.end()), v.erase(unique(v.begin(), v.end()), v.end());
   for (int i= s.size(); i--;) s_cpy[i]= std::lower_bound(v.begin(), v.end(), s[i]) - v.begin() + 1;
   sa= sa_is(s_cpy, v.size() + 1), sa.erase(sa.begin());
  }
 }
 int operator[](int i) const { return sa[i]; }
 size_t size() const { return sa.size(); }
 auto begin() const { return sa.begin(); }
 auto end() const { return sa.end(); }
 // return {l,r} s.t. P is a prefix of S[sa[i]:] ( i in [l,r) )
 // l == r if P is not a substring of S
 // O(|P|log|S|)
 std::pair<int, int> pattern_matching(const String &P) const {
  const int n= s.size(), m= P.size();
  if (n < m) return {0, 0};
  auto f1= [&](int h) {
   auto t= s.begin() + h;
   for (int j= 0, e= std::min(n - h, m); j < e; ++j) {
    if (t[j] < P[j]) return true;
    if (t[j] > P[j]) return false;
   }
   return n - h < m;
  };
  auto f2= [&](int h) {
   auto t= s.begin() + h;
   for (int j= 0, e= std::min(n - h, m); j < e; ++j)
    if (t[j] > P[j]) return false;
   return true;
  };
  auto L= std::partition_point(sa.begin(), sa.end(), f1), R= std::partition_point(L, sa.end(), f2);
  return {L - sa.begin(), R - sa.begin()};
 }
};
struct LCPArray {
 std::vector<int> rnk;
 template <class String> LCPArray(const SuffixArray<String> &sa): rnk(sa.size()) {
  const int n= sa.size(), log= n > 2 ? 31 - __builtin_clz(n - 2) : 0;
  dat.resize(log + 1), dat[0].resize(n - 1);
  auto &lcp= dat[0];
  for (int i= n; i--;) rnk[sa[i]]= i;
  for (int i= 0, h= 0; i < n; ++i) {
   if (rnk[i] == n - 1) {
    h= 0;
    continue;
   }
   for (int j= sa[rnk[i] + 1]; i + h < n && j + h < n && sa.s[i + h] == sa.s[j + h];) ++h;
   if ((lcp[rnk[i]]= h)) --h;
  }
  for (int i= 0, I= 1, j; i < log; ++i, I<<= 1)
   for (dat[i + 1].resize(j= dat[i].size() - I); j--;) dat[i + 1][j]= std::min(dat[i][j], dat[i][j + I]);
 }
 int operator[](int i) const { return dat[0][i]; }
 size_t size() const { return dat[0].size(); }
 auto begin() const { return dat[0].begin(); }
 auto end() const { return dat[0].end(); }
 int operator()(int i, int j) const {
  if (i == j) return rnk.size() - i;
  auto [l, r]= std::minmax(rnk[i], rnk[j]);
  if (r == l + 1) return dat[0][l];
  int k= 31 - __builtin_clz(r - l - 1);
  return std::min(dat[k][l], dat[k][r - (1 << k)]);
 }
private:
 std::vector<std::vector<int>> dat;
};
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>
#include <type_traits>
#define _DETECT_BOOL(name, ...) \
 template <class, class= void> struct name: std::false_type {}; \
 template <class T> struct name<T, std::void_t<__VA_ARGS__>>: std::true_type {}; \
 template <class T> static constexpr bool name##_v= name<T>::value
#define _DETECT_TYPE(name, type1, type2, ...) \
 template <class T, class= void> struct name { \
  using type= type2; \
 }; \
 template <class T> struct name<T, std::void_t<__VA_ARGS__>> { \
  using type= type1; \
 }

template <class M> class SegmentTree {
 _DETECT_BOOL(monoid, typename T::T, decltype(&T::op), decltype(&T::ti));
 _DETECT_BOOL(dual, typename T::T, typename T::E, decltype(&T::mp), decltype(&T::cp));
 _DETECT_TYPE(nullptr_or_E, typename T::E, std::nullptr_t, typename T::E);
 using T= typename M::T;
 using E= typename nullptr_or_E<M>::type;
 int n;
 std::unique_ptr<T[]> dat;
 std::unique_ptr<E[]> laz;
 std::unique_ptr<bool[]> flg;
 inline void update(int k) { dat[k]= M::op(dat[k << 1], dat[k << 1 | 1]); }
 inline bool map(int k, E x, int sz) {
  if constexpr(std::is_invocable_r_v<bool, decltype(M::mp), T&, E, int>) return M::mp(dat[k], x, sz);
  else if constexpr(std::is_invocable_r_v<bool, decltype(M::mp), T&, E>) return M::mp(dat[k], x);
  else if constexpr(std::is_invocable_r_v<void, decltype(M::mp), T&, E, int>) return M::mp(dat[k], x, sz), true;
  else return M::mp(dat[k], x), true;
 }
 inline void prop(int k, E x, int sz) {
  if(k < n) {
   if(flg[k]) M::cp(laz[k], x);
   else laz[k]= x;
   flg[k]= true;
   if constexpr(monoid_v<M>)
    if(!map(k, x, sz)) push(k, sz), update(k);
  } else {
   if constexpr(monoid_v<M>) map(k, x, 1);
   else map(k - n, x, 1);
  }
 }
 inline void push(int k, int sz) {
  if(flg[k]) prop(k << 1, laz[k], sz >> 1), prop(k << 1 | 1, laz[k], sz >> 1), flg[k]= false;
 }
 inline bool valid(int k) const {
  int d= __builtin_clz(k) - __builtin_clz(n);
  return (n >> d) != k || ((n >> d) << d) == n;
 }
public:
 SegmentTree() {}
 SegmentTree(int n): n(n), dat(std::make_unique<T[]>(n << monoid_v<M>)) {
  if constexpr(monoid_v<M>) std::fill_n(dat.get(), n << 1, M::ti());
  if constexpr(dual_v<M>) laz= std::make_unique<E[]>(n), flg= std::make_unique<bool[]>(n), std::fill_n(flg.get(), n, false);
 }
 template <class F> SegmentTree(int n, const F& init): n(n), dat(std::make_unique<T[]>(n << monoid_v<M>)) {
  auto a= dat.get() + (n & -monoid_v<M>);
  for(int i= 0; i < n; ++i) a[i]= init(i);
  if constexpr(monoid_v<M>) build();
  if constexpr(dual_v<M>) laz= std::make_unique<E[]>(n), flg= std::make_unique<bool[]>(n), std::fill_n(flg.get(), n, false);
 }
 SegmentTree(int n, T x): SegmentTree(n, [x](int) { return x; }) {}
 SegmentTree(const std::vector<T>& v): SegmentTree(v.size(), [&v](int i) { return v[i]; }) {}
 SegmentTree(const T* bg, const T* ed): SegmentTree(ed - bg, [bg](int i) { return bg[i]; }) {}
 void build() {
  static_assert(monoid_v<M>, "\"build\" is not available\n");
  for(int i= n; --i;) update(i);
 }
 inline void unsafe_set(int i, T x) {
  static_assert(monoid_v<M>, "\"unsafe_set\" is not available\n");
  dat[i + n]= x;
 }
 inline void set(int i, T x) {
  get(i);
  if constexpr(monoid_v<M>)
   for(dat[i+= n]= x; i>>= 1;) update(i);
  else dat[i]= x;
 }
 inline void mul(int i, T x) {
  static_assert(monoid_v<M>, "\"mul\" is not available\n");
  set(i, M::op(get(i), x));
 }
 inline T get(int i) {
  i+= n;
  if constexpr(dual_v<M>)
   for(int j= 31 - __builtin_clz(i); j; --j) push(i >> j, 1 << j);
  if constexpr(monoid_v<M>) return dat[i];
  else return dat[i - n];
 }
 inline T operator[](int i) { return get(i); }
 inline T prod(int l, int r) {
  static_assert(monoid_v<M>, "\"prod\" is not available\n");
  l+= n, r+= n;
  if constexpr(dual_v<M>) {
   for(int j= 31 - __builtin_clz(l); ((l >> j) << j) != l; --j) push(l >> j, 1 << j);
   for(int j= 31 - __builtin_clz(r); ((r >> j) << j) != r; --j) push(r >> j, 1 << j);
  }
  T s1= M::ti(), s2= M::ti();
  for(; l < r; l>>= 1, r>>= 1) {
   if(l & 1) s1= M::op(s1, dat[l++]);
   if(r & 1) s2= M::op(dat[--r], s2);
  }
  return M::op(s1, s2);
 }
 inline void apply(int l, int r, E x) {
  static_assert(dual_v<M>, "\"apply\" is not available\n");
  l+= n, r+= n;
  for(int j= 31 - __builtin_clz(l); ((l >> j) << j) != l; j--) push(l >> j, 1 << j);
  for(int j= 31 - __builtin_clz(r); ((r >> j) << j) != r; j--) push(r >> j, 1 << j);
  for(int a= l, b= r, sz= 1; a < b; a>>= 1, b>>= 1, sz<<= 1) {
   if(a & 1) prop(a++, x, sz);
   if(b & 1) prop(--b, x, sz);
  }
  if constexpr(monoid_v<M>) {
   for(int j= __builtin_ctz(l) + 1; l >> j; ++j) update(l >> j);
   for(int j= __builtin_ctz(r) + 1; r >> j; ++j) update(r >> j);
  }
 }
 template <class C> int max_right(int l, const C& check) {
  static_assert(monoid_v<M>, "\"max_right\" is not available\n");
  assert(check(M::ti()));
  if(check(prod(l, n))) return n;
  T s= M::ti(), t;
  int sz= 1;
  for(get(l), l+= n;; s= t, ++l) {
   while(!(l & 1) && valid(l >> 1)) l>>= 1, sz<<= 1;
   if(!check(t= M::op(s, dat[l]))) {
    while(l < n) {
     if constexpr(dual_v<M>) push(l, sz);
     l<<= 1, sz>>= 1;
     if(check(t= M::op(s, dat[l]))) s= t, ++l;
    }
    return l - n;
   }
  }
 }
 template <class C> int min_left(int r, const C& check) {
  static_assert(monoid_v<M>, "\"min_left\" is not available\n");
  assert(check(M::ti()));
  if(check(prod(0, r))) return 0;
  T s= M::ti(), t;
  int sz= 1;
  for(get(--r), r+= n;; s= t, --r) {
   while(!valid(r)) r= r << 1 | 1, sz>>= 1;
   while((r & 1) && valid(r >> 1)) r>>= 1, sz<<= 1;
   if(!check(t= M::op(dat[r], s))) {
    while(r < n) {
     if constexpr(dual_v<M>) push(r, sz);
     r= r << 1 | 1, sz>>= 1;
     if(check(t= M::op(dat[r], s))) s= t, --r;
    }
    return r + 1 - n;
   }
  }
 }
};

using namespace std;
struct RMQ {
 using T= int;
 static T ti() { return -(1 << 30); }
 static T op(T a, T b) { return max(a, b); }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 string S;
 cin >> S;
 SuffixArray sa(S);
 LCPArray lcp(sa);
 int B= 0;
 for(int n= N; n >= B + 1;) n-= ++B;
 vector<pair<int, int>> subs;
 for(int l= 0; l < N; ++l)
  for(int n= 1; n <= B && l + n <= N; ++n) subs.emplace_back(l, n);
 auto cmp= [&](auto a, auto b) {
  int l= lcp(a.first, b.first);
  if(l < min(a.second, b.second)) return S[a.first + l] < S[b.first + l];
  return a.second == b.second ? a.first > b.first : a.second < b.second;
 };
 stable_sort(subs.begin(), subs.end(), cmp);
 SegmentTree<RMQ> seg(N + 1);
 seg.set(0, 0);
 for(auto [l, n]: subs) {
  int r= l + n;
  seg.set(r, max(seg[r], seg.prod(0, l + 1) + 1));
 }
 cout << seg.prod(0, N + 1) << '\n';
 return 0;
}
