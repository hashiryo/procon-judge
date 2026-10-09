// https://atcoder.jp/contests/wtf19-open/tasks/wtf19_c2
// GF2p64 の冪と離散対数。ランプの並びを Σ z^x w^y とみると、操作は z^x w^y (1 + z + w) を足すことなので、w = z + 1 で値を取ると変わらず、
// 元のランプ 1 つの z^X (z + 1)^Y に等しい。原始元 a と b で log をとると X + sY と X + tY (mod 2^64 - 1) が分かるので、連立して解く
#include <iostream>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
using i128= __int128;
constexpr u64 M= ~0ull;  // 乗法群の位数 2^64 - 1
u64 add(u64 a, u64 b) { return u64((__uint128_t(a) + b) % M); }
u64 sub(u64 a, u64 b) { return add(a, M - b); }
u64 mul(u64 a, u64 b) { return u64(__uint128_t(a) * b % M); }
u64 gcd(u64 a, u64 b) { return b ? gcd(b, a % b) : a; }
u64 inv(u64 a) {
 i128 x= 0, y= 1, m= M, r= a;
 while(r) {
  const i128 q= m / r;
  m-= q * r, swap(m, r), x-= q * y, swap(x, y);
 }
 return u64((x % M + M) % M);
}
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 static constexpr long long OFS= 1e17;
 const GF2p64 a= GF2p64::generator(), one(1);
 const u64 s= (a + one).log(a);
 GF2p64 b;
 u64 t= 0;
 for(u64 k= 11;; k+= 2) {  // b = a^k も原始元で、s - t が法 2^64 - 1 で割れるもの
  if(gcd(k, M) != 1) continue;
  b= a.pow(k), t= (b + one).log(b);
  if(gcd(sub(s, t), M) == 1) break;
 }
 int N;
 cin >> N;
 vector<u64> x(N), y(N);
 for(int i= 0; i < N; i++) {
  long long xi, yi;
  cin >> xi >> yi;
  x[i]= xi + OFS, y[i]= yi + OFS;
 }
 auto f= [&](GF2p64 z) {
  GF2p64 sum;
  for(int i= 0; i < N; i++) sum+= z.pow(x[i]) * (z + one).pow(y[i]);
  return sum;
 };
 const u64 u= f(a).log(a), v= f(b).log(b), Y= mul(sub(u, v), inv(sub(s, t))), X= sub(u, mul(s, Y));
 cout << (long long)X - OFS << " " << (long long)Y - OFS << '\n';
 return 0;
}
