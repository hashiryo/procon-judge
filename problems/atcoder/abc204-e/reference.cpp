// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-28、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cassert>
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
enum MinMaxEnum { MAXIMIZE= -1, MINIMIZE= 1 };
// [l,r]
template <MinMaxEnum obj, class F> std::pair<int64_t, result_type_t<F>> fibonacci_search(const F& f, int64_t l, int64_t r) {
 assert(l <= r);
 int64_t s= 1, t= 2, a= l - 1, x, b, y;
 for(int64_t e= r - l + 2; t < e;) std::swap(s+= t, t);
 b= a + t, x= b - s;
 result_type_t<F> fx= f(x), fy;
 for(bool g; a + b != 2 * x;) {
  if(y= a + b - x; r < y) b= a, a= y;
  else {
   if constexpr(obj == MINIMIZE) g= fx < (fy= f(y));
   else g= fx > (fy= f(y));
   if(g) b= a, a= y;
   else a= x, x= y, fx= fy;
  }
 }
 return {x, fx};
}

#include <string>
#include <limits>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <cstdint>
template <class Int> constexpr int bsf(Int a) {
 if constexpr (sizeof(Int) == 16) {
  uint64_t lo= a & uint64_t(-1);
  return lo ? __builtin_ctzll(lo) : 64 + __builtin_ctzll(a >> 64);
 } else if constexpr (sizeof(Int) == 8) return __builtin_ctzll(a);
 else return __builtin_ctz(a);
}
template <class Int> constexpr Int binary_gcd(Int a, Int b) {
 if (a == 0 || b == 0) return a + b;
 int n= bsf(a), m= bsf(b), s= 0;
 for (a>>= n, b>>= m; a != b;) {
  Int d= a - b;
  bool f= a > b;
  s= bsf(d), b= f ? b : a, a= (f ? d : -d) >> s;
 }
 return a << std::min(n, m);
}
template <class Int, bool reduction= true> struct Rational {
 Int num, den;
 constexpr Rational(): num(0), den(1) {}
 constexpr Rational(Int n, Int d= 1): num(n), den(d) {
  if(den < 0) num= -num, den= -den;
  if constexpr(reduction) reduce(num, den);
 }
 constexpr Rational(const std::string& str) {
  auto it= str.find("/");
  if(it == std::string::npos) num= std::stoi(str), den= 1;
  else num= std::stoi(str.substr(0, it)), den= std::stoi(str.substr(it + 1));
  if constexpr(reduction) reduce(num, den);
 }
 static constexpr void reduce(Int& a, Int& b) {
  const Int g= binary_gcd(a < 0 ? -a : a, b);
  a/= g, b/= g;
 }
 static constexpr Rational raw(Int n, Int d) {
  Rational ret;
  return ret.num= n, ret.den= d, ret;
 }
 constexpr Rational operator-() const { return raw(-num, den); }
 constexpr Rational operator+(const Rational& r) const { return Rational(num * r.den + den * r.num, den * r.den); }
 constexpr Rational operator-(const Rational& r) const { return Rational(num * r.den - den * r.num, den * r.den); }
 constexpr Rational operator*(const Rational& r) const {
  if constexpr(reduction) {
   Int ln= num, ld= den, rn= r.num, rd= r.den;
   return reduce(ln, rd), reduce(rn, ld), raw(ln * rn, ld * rd);
  } else return Rational(num * r.num, den * r.den);
 }
 constexpr Rational operator/(const Rational& r) const {
  if constexpr(reduction) {
   Int ln= num, ld= den, rn= r.num, rd= r.den;
   if(rn < 0) rd= -rd, rn= -rn;
   return reduce(ln, rn), reduce(rd, ld), raw(ln * rd, ld * rn);
  } else return Rational(num * r.den, den * r.num);
 }
 Rational& operator+=(const Rational& r) { return *this= *this + r; }
 Rational& operator-=(const Rational& r) { return *this= *this - r; }
 Rational& operator*=(const Rational& r) { return *this= *this * r; }
 Rational& operator/=(const Rational& r) { return *this= *this / r; }
 constexpr bool operator==(const Rational& r) const {
  if constexpr(reduction) return num == r.num && den == r.den;
  else return den == 0 && r.den == 0 ? num * r.num > 0 : num * r.den == den * r.num;
 }
 constexpr bool operator!=(const Rational& r) const { return !(*this == r); }
 constexpr bool operator<(const Rational& r) const {
  if(den == 0 && r.den == 0) return num < r.num;
  else if(den == 0) return num < 0;
  else if(r.den == 0) return r.num > 0;
  else return num * r.den < den * r.num;
 }
 constexpr bool operator>(const Rational& r) const { return r < *this; }
 constexpr bool operator<=(const Rational& r) const { return !(r < *this); }
 constexpr bool operator>=(const Rational& r) const { return !(*this < r); }
 constexpr explicit operator bool() const { return num != 0; }
 constexpr long double to_fp() const { return (long double)num / den; }
 constexpr explicit operator long double() const { return to_fp(); }
 constexpr explicit operator double() const { return to_fp(); }
 constexpr explicit operator float() const { return to_fp(); }
 constexpr Int floor() const { return num < 0 ? -((-num + den - 1) / den) : num / den; }
 constexpr Int ceil() const { return num < 0 ? -(-num / den) : (num + den - 1) / den; }
 constexpr Rational abs() const { return raw(num < 0 ? -num : num, den); }
 constexpr friend Int floor(const Rational& r) { return r.floor(); }
 constexpr friend Int ceil(const Rational& r) { return r.ceil(); }
 constexpr friend Rational abs(const Rational& r) { return r.abs(); }
 std::string to_string() const {
  if(!num) return "0";
  std::stringstream ss;
  if(den == 1) return ss << num, ss.str();
  return ss << num << "/" << den, ss.str();
 }
 friend std::istream& operator>>(std::istream& is, Rational& r) {
  std::string s;
  if(is >> s; s != "") r= Rational(s);
  return is;
 }
 friend std::ostream& operator<<(std::ostream& os, const Rational& r) { return os << r.to_string(); }
};
template <class Int, bool reduction> struct std::numeric_limits<Rational<Int, reduction>> {
 static constexpr Rational<Int, reduction> max() noexcept { return Rational<Int, reduction>(1, 0); }
 static constexpr Rational<Int, reduction> min() noexcept { return Rational<Int, reduction>(1, std::numeric_limits<Int>::max()); }
 static constexpr Rational<Int, reduction> lowest() noexcept { return Rational<Int, reduction>(-1, 0); }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M;
 cin >> N >> M;
 int A[M], B[M];
 long long C[M], D[M];
 vector<int> adj[N];
 for(int i= 0; i < M; ++i) {
  cin >> A[i] >> B[i] >> C[i] >> D[i], --A[i], --B[i];
  adj[A[i]].push_back(i), adj[B[i]].push_back(i);
 }
 priority_queue<pair<long long, int>> pq;
 static constexpr long long INF= 1e18;
 long long dist[N];
 fill_n(dist, N, INF);
 dist[0]= 0, pq.emplace(0, 0);
 while(!pq.empty()) {
  auto [d, u]= pq.top();
  pq.pop();
  d= -d;
  if(dist[u] != d) continue;
  for(auto e: adj[u]) {
   int v= A[e] ^ B[e] ^ u;
   auto f= [&](long long t) { return Rational<__int128, false>((__int128)(t + C[e]) * (t + 1) + D[e], t + 1); };
   auto [_, nd_f]= fibonacci_search<MINIMIZE>(f, d, max(d, D[e]));
   long long nd= floor(nd_f);
   if(dist[v] > nd) dist[v]= nd, pq.emplace(-nd, v);
  }
 }
 cout << (dist[N - 1] == INF ? -1 : dist[N - 1]) << '\n';
 return 0;
}
