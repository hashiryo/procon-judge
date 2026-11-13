// lib.cpp と同じ二分探索と DP を NeoLibrary の DFA (neo/counting/DFA.hpp) で回す。N 以下の DFA は leq_digits にする。
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include "neo/counting/DFA.hpp"
using namespace std;
constexpr int CIRC[10]= {1, 0, 0, 0, 1, 0, 1, 0, 2, 1};

// [0, N] の数の「丸の総数」。T = (始まっていない数, 始まった数, 始まった数の丸の和) で、先頭の 0 は数えない
static long long getK(long long N) {
 if(N <= 0) return 0;
 string s= to_string(N);
 int n= (int)s.size();
 auto dfa= leq_digits(s);
 using T= array<__int128, 3>;
 auto op= [](const T& l, const T& r) -> T { return T{l[0] + r[0], l[1] + r[1], l[2] + r[2]}; };
 auto f= [&](const T& v, int d, int) -> T {
  __int128 ncu= (d == 0) ? v[0] : 0;
  __int128 ncs= v[1] + (d != 0 ? v[0] : 0);
  __int128 ns= v[2] + (__int128)CIRC[d] * ncs;
  return T{ncu, ncs, ns};
 };
 T r= dfa.dp(n, op, T{0, 0, 0}, f, T{1, 0, 0});
 if(r[2] > (__int128)9'000'000'000'000'000'000LL) return 9'000'000'000'000'000'000LL;
 return (long long)r[2];
}

signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 long long K;
 cin >> K;
 long long ng= 0, ok= 1'000'000'000'000'000'000LL + 1;
 while(ok - ng > 1) {
  long long mid= ng + (ok - ng) / 2;
  if(getK(mid) >= K) ok= mid;
  else ng= mid;
 }
 if(getK(ok) == K) cout << ok << '\n';
 else cout << -1 << '\n';
 return 0;
}
