// 期待出力を作る参照実装。submissions/lib-segdual.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <array>
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
using Mint= ModInt<998244353>;
struct Mono {
 struct T {
  Mint val, coef[2];
  T()= default;
  T(Mint id, Mint v): val(v), coef{(id + 1) * (id + 2) / 2, (id * 2 + 3) / 2} {}
 };
 using E= array<Mint, 3>;
 static void mp(T& x, const E& mapp) { x.val+= mapp[0] * x.coef[0] - mapp[1] * x.coef[1] + mapp[2]; }
 static void cp(E& pre, const E& suf) { pre[0]+= suf[0], pre[1]+= suf[1], pre[2]+= suf[2]; }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, Q;
 cin >> N >> Q;
 Mint A[N], D[N];
 for(int i= 0; i < N; i++) cin >> A[i], D[i]= A[i];
 for(int j= 0; j < 3; j++)
  for(int i= 1; i < N; i++) D[i]+= D[i - 1];
 SegmentTree<Mono> seg(N);
 for(int i= 0; i < N; i++) seg.set(i, {i, D[i]});
 while(Q--) {
  int op, x;
  cin >> op >> x, x--;
  if(op == 1) {
   Mint v;
   cin >> v, v-= A[x], A[x]+= v;
   seg.apply(x, N, {v, v * x, v * x * x / 2});
  } else {
   cout << seg[x].val << '\n';
  }
 }
 return 0;
}
