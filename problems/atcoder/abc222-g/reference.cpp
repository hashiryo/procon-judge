// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
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

class Montgomery32 {};  // mod < 2^32 & mod is odd
class Montgomery64 {};  // mod < 2^62 & mod is odd
class Barrett {};       // 2^20 < mod <= 2^41
namespace math_internal {
struct r_b: m_b {};
}
template <class mod_t> constexpr bool is_runtimemodint_v= std::is_base_of_v<math_internal::r_b, mod_t>;
namespace math_internal {
template <class MP, u64 M, int id> struct RB: r_b {
 static inline void set_mod(u64 m) { assert(m <= M), md= MP(m); }
 static inline u64 max() { return M; }
protected:
 static inline MP md;
};
template <class T, typename= enable_if_t<is_runtimemodint_v<T>>> constexpr u64 mv() { return T::max(); }
template <class Int, int id= -1> using ModInt_Runtime= conditional_t<is_same_v<Int, int>, MInt<u32, RB<MP_Na, u32(-1), id>>, conditional_t<is_same_v<Int, u32>, MInt<u32, RB<MP_Na, 0xFFFFFFFF, id>>, conditional_t<is_same_v<Int, long long>, MInt<u64, RB<MP_D2B1_1, (1ull << 63) - 1, id>>, conditional_t<is_same_v<Int, Montgomery32>, MInt<u32, RB<MP_Mo32, (1 << 30) - 1, id>>, conditional_t<is_same_v<Int, Montgomery64>, MInt<u64, RB<MP_Mo64, (1ull << 62) - 1, id>>, conditional_t<is_same_v<Int, Barrett>, MInt<u64, RB<MP_Br, 1ull << 41, id>>, MInt<u64, RB<MP_D2B1_2, u64(-1), id>>>>>>>>;
}
using math_internal::ModInt_Runtime;

#include <cmath>
#include <vector>
#include <type_traits>
#include <tuple>
// clang-format off
namespace function_template_internal{
template<class C>struct is_function_object{
 template<class U,int dummy=(&U::operator(),0)> static std::true_type check(U *);
 static std::false_type check(...);
 static C *m;
 static constexpr bool value= decltype(check(m))::value;
};
template<class F,bool,bool>struct function_type_impl{using type= void;};
template<class F>struct function_type_impl<F,true,false>{using type= F *;};
template<class F>struct function_type_impl<F,false,true>{using type= decltype(&F::operator());};
template<class F> using function_type_t= typename function_type_impl<F,std::is_function_v<F>,is_function_object<F>::value>::type;
template<class... Args>struct result_type_impl{using type= void;};
template<class R,class... Args>struct result_type_impl<R(*)(Args...)>{using type= R;};
template<class C,class R,class... Args>struct result_type_impl<R(C::*)(Args...)>{using type= R;};
template<class C,class R,class... Args>struct result_type_impl<R(C::*)(Args...)const>{using type= R;};
template<class F> using result_type_t= typename result_type_impl<function_type_t<F>>::type;
template<class... Args>struct argument_type_impl{using type= void;};
template<class R,class... Args>struct argument_type_impl<R(*)(Args...)>{using type= std::tuple<Args...>;};
template<class C,class R,class... Args>struct argument_type_impl<R(C::*)(Args...)>{using type= std::tuple<Args...>;};
template<class C,class R,class... Args>struct argument_type_impl<R(C::*)(Args...)const>{using type= std::tuple<Args...>;};
template<class F> using argument_type_t= typename argument_type_impl<function_type_t<F>>::type;
}
using function_template_internal::result_type_t,function_template_internal::argument_type_t;
// clang-format on
// mp : E × T -> T
// op : E × E -> E
// hash : T -> int
// s,t ∈ T, x ∈ E
// return min{ i : x^i(s) = t and i ∈ [0,N) } or -1 (not found)
template <class F, class G, class H> class DiscreteLogarithm {
 const F& mp;
 const G& op;
 const H& hash;
 const int64_t lim;
 using T= result_type_t<F>;
 using E= result_type_t<G>;
public:
 DiscreteLogarithm(const F& mp, const G& op, const H& hash, int64_t lim= 1ll << 50): mp(mp), op(op), hash(hash), lim(lim) { static_assert(std::is_convertible_v<std::invoke_result_t<H, T>, int>); }
 int64_t operator()(const E& x, T s, const T& t, int64_t N= -1) const {
  if(N < 0) N= lim;
  const int m= 1 << (31 - __builtin_clz(int(std::sqrt(N) + 1))), mask= m - 1;
  std::vector<T> val(m), vs(m);
  std::vector<int> os(m + 1), so(m);
  T s1= t;
  for(int i= 0; i < m; ++i) ++os[so[i]= hash(val[i]= s1= mp(x, s1)) & mask];
  for(int i= 0; i < m; ++i) os[i + 1]+= os[i];
  for(int i= 0; i < m; ++i) vs[--os[so[i]]]= val[i];
  E y= x;
  for(int k= m; k>>= 1;) y= op(y, y);
  bool failed= false;
  for(int64_t n= 0;; s= s1) {
   for(int a= hash(s1= mp(y, s)) & mask, j= os[a]; j < os[a + 1]; ++j) {
    if(s1 == vs[j]) {
     for(int i= 0;; s= mp(x, s)) {
      if(s == t) return n + i < N ? n + i : -1;
      if(++i == m) break;
     }
     if(failed) return -1;
     failed= true;
     break;
    }
   }
   if((n+= m) >= N) break;
  }
  return -1;
 }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt_Runtime<int>;
 using Aff= array<Mint, 2>;
 auto mp= [](Aff f, Mint x) { return f[0] * x + f[1]; };
 auto op= [](Aff l, Aff r) { return Aff{l[0] * r[0], l[0] * r[1] + l[1]}; };
 DiscreteLogarithm log(mp, op, [](Mint x) { return x.val(); }, 1e8);
 int T;
 cin >> T;
 while(T--) {
  int K;
  cin >> K;
  Mint::set_mod(K);
  int ans= log({10, 2}, 2, 0) + 1;
  cout << (ans ? ans : -1) << '\n';
 }
 return 0;
}
