// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// Alien DP
#include <iostream>
#include <algorithm>
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
 ios::sync_with_stdio(false);
 int N, R;
 cin >> N >> R;
 if(R > N - R) R= N - R;
 long long A[N - 1];
 for(int i= 0; i < N - 1; ++i) cin >> A[i];
 long long B[N];
 fill_n(B, N, 0);
 for(int i= 0; i < N - 1; ++i) B[i]+= A[i], B[i + 1]+= A[i];
 auto f= [&](long long p) {
  long long dp[N + 1][2];
  dp[0][0]= 0, dp[0][1]= -1e18;
  for(int i= 0; i < N; ++i) {
   dp[i + 1][1]= dp[i][0] + B[i] - p;
   dp[i + 1][0]= max(dp[i][0], dp[i][1]);
  }
  return max(dp[N][0], dp[N][1]) + p * R;
 };
 auto a= *max_element(B, B + N);
 auto [_, ans]= fibonacci_search<MINIMIZE>(f, -3 * a, 3 * a);
 cout << ans << '\n';
 return 0;
}
