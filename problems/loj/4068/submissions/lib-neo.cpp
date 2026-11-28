// LOJ 4068「GDKOI-S 2023」错排。
// T 個の (n,m) に答える。長さ n の順列 p で、i≤m なら p_i>m、かつすべての i で p_i≠i の数。
// 包除で f(n,m)=P(k,m)・E(k,m) (k=n-m, E(k,m)=Σ_j C(m,j)!(k-j)) と書けるが、ここでは
// パラメータ空間 (n,m) の Mo で、隣の点へ O(1) で移る漸化式を使って解く。
//   f(n+1,m) = (n-m)(n-m+1)/(n-2m+1)・(f(n,m)+f(n-1,m))   (n≥2m)
//   f(n,m+1) = (n-2m)/(n-m)・f(n,m) - m・f(n-1,m)          (m<n)
//   f(n-1,m+1) = d・f(n-1,m) - e・f(n,m)                    (d=(n-1-2m)/(n-1-m)+m, e=m(n-2m)/((n-m-1)(n-m)))
// 状態は (f(n,m), f(n-1,m)) の 2 つ。m を減らすときは下の 2 本を連立して逆に解く。
// 2m>n では f=0、n=2m では f=(m!)² が境界の値になる。
// Mo には (l,r)=(m,n) を渡す (m≤n)。並べ方は m のブロックと n の交互。
#include <cstdio>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
using i64 = long long;
static constexpr i64 MOD= 998244353;

signed main() {
 int T;
 scanf("%d", &T);
 vector<int> qn(T), qm(T);
 int maxn= 0;
 for(int i= 0; i < T; ++i) {
  scanf("%d %d", &qn[i], &qm[i]);
  if(qn[i] > maxn) maxn= qn[i];
 }
 const int LIM= maxn + 5;
 vector<i64> fact(LIM), inv(LIM);
 fact[0]= 1;
 for(int i= 1; i < LIM; ++i) fact[i]= fact[i - 1] * i % MOD;
 inv[0]= 0;
 if(LIM > 1) inv[1]= 1;
 for(int i= 2; i < LIM; ++i) inv[i]= (MOD - (MOD / i) * inv[MOD % i] % MOD) % MOD;
 // 状態: f(n,m) と f(n-1,m)。初期値は f(0,0)=1, f(-1,0)=0。
 int cn= 0, cm= 0;
 i64 F0= 1, F1= 0;
 auto n_up= [&](int) {
  int N= cn, M= cm;
  if(2LL * M > N + 1) F0= F1= 0;
  else if(2LL * M == N + 1) F0= fact[M] * fact[M] % MOD, F1= 0;
  else {
   i64 C= (i64)(N - M) % MOD * ((N - M + 1) % MOD) % MOD * inv[N - 2 * M + 1] % MOD;
   i64 nf= C * ((F0 + F1) % MOD) % MOD;
   F1= F0, F0= nf;
  }
  ++cn;
 };
 auto n_down= [&](int) {
  int N= cn, M= cm;
  if(2LL * M > N - 1) F0= F1= 0;
  else if(2LL * M == N - 1) F0= fact[M] * fact[M] % MOD, F1= 0;
  else {
   // C=(N-M-1)(N-M)/(N-2M) を掛けて足し戻す形で f(N-2,M) を出す。
   i64 cinv= inv[N - M - 1] * inv[N - M] % MOD * ((N - 2 * M) % MOD) % MOD;
   i64 f2= (F0 * cinv % MOD - F1 + MOD) % MOD;
   F0= F1, F1= f2;
  }
  --cn;
 };
 auto m_up= [&](int) {
  int N= cn, M= cm, M1= M + 1;
  i64 nF0= 0, nF1= 0;
  if(2LL * M1 <= N) {
   i64 a= (i64)(N - 2 * M) % MOD * inv[N - M] % MOD;
   nF0= (a * F0 - (i64)M * F1) % MOD;
   if(nF0 < 0) nF0+= MOD;
  }
  if(2LL * M1 <= N - 1) {
   i64 dnum= (i64)(N - 1 - 2 * M) + (i64)M * (N - 1 - M);
   i64 d= (dnum % MOD + MOD) % MOD * inv[N - M - 1] % MOD;
   i64 e= (i64)M % MOD * ((N - 2 * M) % MOD) % MOD * inv[N - M - 1] % MOD * inv[N - M] % MOD;
   nF1= (d * F1 - e * F0) % MOD;
   if(nF1 < 0) nF1+= MOD;
  }
  F0= nF0, F1= nF1, ++cm;
 };
 auto m_down= [&](int) {
  int N= cn, M= cm, M1= M - 1;
  if(2LL * M1 > N) F0= F1= 0;
  else if(2LL * M1 == N) F0= fact[M1] * fact[M1] % MOD, F1= 0;
  else if(N == 1) F0= 0, F1= 1;
  else if((i64)N == 2LL * M - 1) {
   // f(2M-1,M-1)=M!・(M!-(M-1)!), f(2M-2,M-1)=((M-1)!)²
   i64 fm= fact[M], fm1= fact[M1];
   F0= fm * ((fm - fm1 + MOD) % MOD) % MOD;
   F1= fm1 * fm1 % MOD;
  } else {
   // 連立: F0 = a x - M1 y, F1 = d y - e x を x,y について解く。
   i64 y= (((i64)(N - M) % MOD * F1 % MOD + (i64)(M - 1) % MOD * F0 % MOD) % MOD) * inv[M] % MOD * inv[N - 2 * M + 1] % MOD;
   i64 x= (F0 + (i64)(M - 1) % MOD * y % MOD) % MOD * ((N - M + 1) % MOD) % MOD * inv[N - 2 * M + 2] % MOD;
   F0= x, F1= y;
  }
  --cm;
 };
 Mo mo;
 for(int i= 0; i < T; ++i) mo.query(qm[i], qn[i]);
 vector<i64> ans(T);
 auto out= [&](int id) { ans[id]= F0; };
 mo.run(m_down, n_up, m_up, n_down, out);
 for(int i= 0; i < T; ++i) printf("%lld\n", (long long)ans[i]);
}
