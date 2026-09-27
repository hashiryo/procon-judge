// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <iostream>
#include <iterator>
#include <type_traits>
#define _LR(name, IT, CT) \
 template <class T> struct name { \
  using Iterator= typename std::vector<T>::IT; \
  Iterator bg, ed; \
  Iterator begin() const { return bg; } \
  Iterator end() const { return ed; } \
  size_t size() const { return std::distance(bg, ed); } \
  CT &operator[](int i) const { return bg[i]; } \
 }
_LR(ListRange, iterator, T);
_LR(ConstListRange, const_iterator, const T);
#undef _LR
template <class T> struct CSRArray {
 std::vector<T> dat;
 std::vector<int> p;
 size_t size() const { return p.size() - 1; }
 ListRange<T> operator[](int i) { return {dat.begin() + p[i], dat.begin() + p[i + 1]}; }
 ConstListRange<T> operator[](int i) const { return {dat.cbegin() + p[i], dat.cbegin() + p[i + 1]}; }
};
template <template <class> class F, class T> std::enable_if_t<std::disjunction_v<std::is_same<F<T>, ListRange<T>>, std::is_same<F<T>, ConstListRange<T>>, std::is_same<F<T>, CSRArray<T>>>, std::ostream &> operator<<(std::ostream &os, const F<T> &r) {
 os << '[';
 for (int _= 0, __= r.size(); _ < __; ++_) os << (_ ? ", " : "") << r[_];
 return os << ']';
}
namespace nt_internal {
using namespace std;
vector<int> ps, lf;
void sieve(int N) {
 static int n= 2;
 if(n > N) return;
 if(lf.resize((N + 1) >> 1); n == 2) ps.push_back(n++);
 int M= (N - 1) / 2;
 for(int j= 1, e= ps.size(); j < e; ++j) {
  int p= ps[j];
  if(int64_t(p) * p > N) break;
  for(auto k= int64_t(p) * max(n / p / 2 * 2 + 1, p) / 2; k <= M; k+= p) lf[k]+= p * !lf[k];
 }
 for(; n <= N; n+= 2)
  if(!lf[n >> 1]) {
   ps.push_back(lf[n >> 1]= n);
   for(auto j= int64_t(n) * n / 2; j <= M; j+= n) lf[j]+= n * !lf[j];
  }
}
ConstListRange<int> enumerate_primes() { return {ps.cbegin(), ps.cend()}; }
ConstListRange<int> enumerate_primes(int N) {
 sieve(N);
 return {ps.cbegin(), upper_bound(ps.cbegin(), ps.cend(), N)};
}
int least_prime_factor(int n) { return n & 1 ? sieve(n), lf[(n >> 1)] : 2; }
// f(p,e) := f(p^e)
template <class T, class F> vector<T> completely_multiplicative_table(int N, const F& f) {
 vector<T> ret(N + 1);
 sieve(N);
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lf[i] == n ? f(n, 1) : ret[lf[i]] * ret[n / lf[i]];
 if(int n= 4; 2 <= N)
  for(T t= ret[2]= f(2, 1); n <= N; n+= 2) ret[n]= t * ret[n >> 1];
 return ret[1]= 1, ret;
}
}
using nt_internal::enumerate_primes, nt_internal::least_prime_factor, nt_internal::completely_multiplicative_table;
// O(N log k / log N + N)
template <class T> static std::vector<T> pow_table(int N, uint64_t k) {
 if(k == 0) return std::vector<T>(N + 1, 1);
 auto f= [k](int p, int) {
  T ret= 1, b= p;
  for(auto e= k;; b*= b) {
   if(e & 1) ret*= b;
   if(!(e>>= 1)) return ret;
  }
 };
 return completely_multiplicative_table<T>(N, f);
}

namespace nt_internal {
vector<int> lfw;
vector<char> lfe;
void set_lfe(int N) {
 static int n= 3, i= 1;
 if(n > N) return;
 for(sieve(N), lfw.resize((N + 1) >> 1), copy(lf.begin() + i, lf.begin() + ((N + 1) >> 1), lfw.begin() + i), lfe.resize(((N + 1) >> 1), 1); n <= N; n+= 2, ++i)
  if(int j= (n / lf[i]) >> 1; lf[i] == lf[j]) lfe[i]+= lfe[j], lfw[i]*= lfw[j];
}
// O(log n)
vector<pair<int, short>> factorize(int n) {
 vector<pair<int, short>> ret;
 if(short t; !(n & 1)) ret.emplace_back(2, t= __builtin_ctz(n)), n>>= t;
 if(int i= n >> 1; n > 1)
  for(set_lfe(n); n > 1; i= n >> 1) ret.emplace_back(lf[i], lfe[i]), n/= lfw[i];
 return ret;
}
// f(p,e) := f(p^e)
template <class T, class F> vector<T> multiplicative_table(int N, const F& f) {
 vector<T> ret(N + 1);
 set_lfe(N);
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? f(lf[i], lfe[i]) : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2, t; n <= N; n+= 2) t= __builtin_ctz(n), ret[n]= n & (n - 1) ? ret[n & -n] * ret[n >> t] : f(2, t);
 return ret[1]= 1, ret;
}
// O(N)
template <class T= int> vector<T> mobius_table(int N) {
 vector<T> ret(N + 1);
 set_lfe(N), ret[1]= 1;
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? -(lfe[i] == 1) : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2; n <= N; n+= 4) ret[n]= -ret[n >> 1];
 return ret;
}
// O(N)
template <class T= int> vector<T> totient_table(int N) {
 vector<T> ret(N + 1);
 set_lfe(N), ret[1]= 1;
 for(int n= 3, i= 1; n <= N; n+= 2, ++i) ret[n]= lfw[i] == n ? lf[i] == n ? n - 1 : ret[n / lf[i]] * lf[i] : ret[lfw[i]] * ret[n / lfw[i]];
 for(int n= 2; n <= N; n+= 4) ret[n]= ret[n >> 1];
 for(int n= 4; n <= N; n+= 4) ret[n]= ret[n >> 1] << 1;
 return ret;
}
}
using nt_internal::factorize, nt_internal::multiplicative_table, nt_internal::mobius_table, nt_internal::totient_table;
// f -> g s.t. g(n) = sum_{m|n} f(m), O(N log log N)
template <typename T> void divisor_zeta(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= 1, e= a.size(); p * j < e; ++j) a[p * j]+= a[j];
}
// a -> h s.t. a(n) = sum_{m|n} h(m), O(N log log N)
template <typename T> void divisor_mobius(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= (a.size() - 1) / p; j; --j) a[p * j]-= a[j];
}
// O(N log log N)
template <typename T> std::vector<T> lcm_convolve(std::vector<T> a, std::vector<T> b) {
 std::size_t N= std::max(a.size(), b.size());
 for(a.resize(N), b.resize(N), divisor_zeta(a), divisor_zeta(b); N--;) a[N]*= b[N];
 return divisor_mobius(a), a;
}
// a -> g s.t. g(n) = sum_{n|m} a(m), O(N log log N)
template <typename T> static void multiple_zeta(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= (a.size() - 1) / p; j; --j) a[j]+= a[p * j];
}
// a -> h s.t. a(n) = sum_{n|m} h(m), O(N log log N)
template <typename T> static void multiple_mobius(std::vector<T>& a) {
 for(auto p: enumerate_primes(a.size() - 1))
  for(int j= 1, e= a.size(); p * j < e; ++j) a[j]-= a[p * j];
}
// O(N log log N)
template <typename T> static std::vector<T> gcd_convolve(std::vector<T> a, std::vector<T> b) {
 std::size_t N= std::max(a.size(), b.size());
 for(a.resize(N), b.resize(N), multiple_zeta(a), multiple_zeta(b); N--;) a[N]*= b[N];
 return multiple_mobius(a), a;
}

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long N;
 cin >> N;
 vector<long long> a(1000'010);
 int g= 0;
 for(int i= 0, A; i < N; ++i) cin >> A, ++a[A], g= gcd(g, A);
 if(g != 1) return cout << "not coprime" << '\n', 0;
 auto b= gcd_convolve(a, a);
 cout << (b[1] - a[1] == N * (N - 1) ? "pairwise" : "setwise") << " coprime" << '\n';
 return 0;
}
