// https://yukicoder.me/problems/no/1901
// 標数 2 では、xor の畳み込みの群環は e_i -> Π_{j ∈ i} (1 + y_j) で R[y_1, ..., y_n] / (y_j^2) と同型になる。
// 上位集合の和で移し、subset convolution で掛けて、上位集合の和で戻す (標数 2 では上位集合の和は自分自身が逆)。
// 係数は次数 31 以下の F_2[x] の元で、その積は次数 62 以下なので x^64 + ... で割ることが起きず、GF2p64 の積がそのまま F_2[x] の積になる
#include <cstdio>
#include <vector>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
static char ibuf[1 << 25];
size_t ipos, ilen;
int read_bit() {
 for(;; ++ipos) {
  if(ipos == ilen) ilen= fread(ibuf, 1, sizeof(ibuf), stdin), ipos= 0;
  if(ibuf[ipos] == '0' || ibuf[ipos] == '1') return ibuf[ipos++] - '0';
 }
}
int read_int() {
 int x= 0;
 for(;; ++ipos) {
  if(ipos == ilen) ilen= fread(ibuf, 1, sizeof(ibuf), stdin), ipos= 0;
  if(ibuf[ipos] >= '0' && ibuf[ipos] <= '9') break;
 }
 for(;; ++ipos) {
  if(ipos == ilen) ilen= fread(ibuf, 1, sizeof(ibuf), stdin), ipos= 0;
  if(ibuf[ipos] < '0' || ibuf[ipos] > '9') return x;
  x= x * 10 + (ibuf[ipos] - '0');
 }
}
signed main() {
 const int n= read_int(), N= 1 << n, R= n + 1;
 vector<GF2p64> F(size_t(N) * R), G(size_t(N) * R), H(size_t(N) * R);  // [S][r]: 要素数 r の部分集合ぶん
 for(auto* P: {&F, &G}) {
  vector<u64> a(N);
  for(int i= 0; i < N; ++i)
   for(int j= 0; j < 32; ++j) a[i]|= u64(read_bit()) << j;
  for(int b= 0; b < n; ++b)  // 上位集合の和
   for(int i= 0; i < N; ++i)
    if(!(i >> b & 1)) a[i]^= a[i | 1 << b];
  for(int S= 0; S < N; ++S) (*P)[size_t(S) * R + __builtin_popcount(S)]= GF2p64(a[S]);
 }
 auto zeta= [&](vector<GF2p64>& P) {  // 部分集合の和 (標数 2 ではメビウス変換も同じ)
  for(int b= 0; b < n; ++b)
   for(int S= 0; S < N; ++S)
    if(S >> b & 1)
     for(int r= 0; r < R; ++r) P[size_t(S) * R + r]+= P[size_t(S ^ 1 << b) * R + r];
 };
 zeta(F), zeta(G);
 for(int S= 0; S < N; ++S) {
  const GF2p64 *f= &F[size_t(S) * R], *g= &G[size_t(S) * R];
  GF2p64* h= &H[size_t(S) * R];
  for(int r= 0; r < R; ++r)
   for(int i= 0; i <= r; ++i) h[r]+= f[i] * g[r - i];
 }
 zeta(H);
 vector<u64> c(N);
 for(int S= 0; S < N; ++S) c[S]= u64(H[size_t(S) * R + __builtin_popcount(S)]);
 for(int b= 0; b < n; ++b)
  for(int i= 0; i < N; ++i)
   if(!(i >> b & 1)) c[i]^= c[i | 1 << b];
 vector<char> out(size_t(N) * 126);
 char* p= out.data();
 for(int i= 0; i < N; ++i)
  for(int j= 0; j < 63; ++j) *p++= char('0' + (c[i] >> j & 1)), *p++= j == 62 ? '\n' : ' ';
 fwrite(out.data(), 1, p - out.data(), stdout);
 return 0;
}
