// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <iomanip>
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

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 long long A, B;
 cin >> A >> B;
 auto f= [&](long long n) { return (long double)B * n + A / sqrt(n + 1); };
 auto [x, fx]= fibonacci_search<MINIMIZE>(f, 0, 1e18);
 cout << fixed << setprecision(15) << fx << '\n';
 return 0;
}
