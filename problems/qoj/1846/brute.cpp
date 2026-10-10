// qoj-1846 の愚直解。a_K, a_{K+1}, ..., a_m を漸化式のとおりに順に計算する。多項式も GF2p64 も使わないので、提出とは別の考え方になる。
// m が 3 × 10^5 以下の入力でだけ使う (gen.py の seed 1000 以上)。nimber の積は、上下に分けて X = 2^(w/2) について
// X^2 = X + X/2 を使う形で計算する (8 bit どうしは表を引く)。
#include <algorithm>
#include <cstdio>
#include <vector>
using u64= unsigned long long;

unsigned char T8[256][256];
u64 mul_rec(u64 a, u64 b, int w) {
 if(w == 1) return a & b;
 const int h= w / 2;
 const u64 m= (u64(1) << h) - 1, a1= a >> h, a0= a & m, b1= b >> h, b0= b & m;
 const u64 c= mul_rec(a0, b0, h), d= mul_rec(a1 ^ a0, b1 ^ b0, h), e= mul_rec(a1, b1, h);
 return (d ^ c) << h | (c ^ mul_rec(e, u64(1) << (h - 1), h));
}
u64 mul(u64 a, u64 b, int w= 32) {
 if(w == 8) return T8[a][b];
 const int h= w / 2;
 const u64 m= (u64(1) << h) - 1, a1= a >> h, a0= a & m, b1= b >> h, b0= b & m;
 const u64 c= mul(a0, b0, h), d= mul(a1 ^ a0, b1 ^ b0, h), e= mul(a1, b1, h);
 return (d ^ c) << h | (c ^ mul(e, u64(1) << (h - 1), h));
}

int main() {
 for(int a= 0; a < 256; ++a)
  for(int b= 0; b < 256; ++b) T8[a][b]= (unsigned char)mul_rec(a, b, 8);
 int K;
 u64 m;
 if(scanf("%d %llu", &K, &m) != 2 || m > 300000) return 1;
 std::vector<u64> a(std::max<u64>(m, K) + 1), b(6), c(6);
 for(int i= 1; i < K; ++i)
  if(scanf("%llu", &a[i]) != 1) return 1;
 for(int i= 1; i <= 5; ++i)
  if(scanf("%llu", &b[i]) != 1) return 1;
 for(int i= 1; i <= 5; ++i)
  if(scanf("%llu", &c[i]) != 1) return 1;
 for(u64 n= K; n <= m; ++n) {
  u64 v= 0;
  for(int i= 1; i <= 5; ++i) v^= mul(a[n - i], b[i]) ^ mul(a[n - K + i], c[i]);
  a[n]= v;
 }
 printf("%llu\n", a[m]);
}
