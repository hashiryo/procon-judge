// cf-1310-f の愚直解。a, a^2, a^3, ... と順に掛けていき、b に当たればその指数を、1 に戻れば -1 を出す。
// 離散対数のアルゴリズムも GF2p64 も使わないので、提出とは別の考え方になる。a の位数が 3 × 10^6 以下の入力でだけ使う
// (gen.py の seed 1000 以上)。nimber の積はチェッカと同じく上下に分ける形で計算する。
#include <cstdio>
using u64= unsigned long long;

unsigned char T8[256][256];
u64 mul_rec(u64 a, u64 b, int w) {
 if(w == 1) return a & b;
 const int h= w / 2;
 const u64 m= (u64(1) << h) - 1, a1= a >> h, a0= a & m, b1= b >> h, b0= b & m;
 const u64 c= mul_rec(a0, b0, h), d= mul_rec(a1 ^ a0, b1 ^ b0, h), e= mul_rec(a1, b1, h);
 return (d ^ c) << h | (c ^ mul_rec(e, u64(1) << (h - 1), h));
}
u64 mul(u64 a, u64 b, int w= 64) {
 if(w == 8) return T8[a][b];
 const int h= w / 2;
 const u64 m= (u64(1) << h) - 1, a1= a >> h, a0= a & m, b1= b >> h, b0= b & m;
 const u64 c= mul(a0, b0, h), d= mul(a1 ^ a0, b1 ^ b0, h), e= mul(a1, b1, h);
 return (d ^ c) << h | (c ^ mul(e, u64(1) << (h - 1), h));
}

int main() {
 for(int a= 0; a < 256; ++a)
  for(int b= 0; b < 256; ++b) T8[a][b]= (unsigned char)mul_rec(a, b, 8);
 int t;
 if(scanf("%d", &t) != 1) return 1;
 while(t--) {
  u64 a, b;
  if(scanf("%llu %llu", &a, &b) != 2) return 1;
  if(b == 1) {
   puts("0");
   continue;
  }
  u64 cur= a, x= 1;
  for(; x <= 3000000 && cur != b && cur != 1; ++x) cur= mul(cur, a);
  if(cur == b) printf("%llu\n", x);
  else if(cur == 1) puts("-1");
  else return 1;  // 位数が大きすぎて愚直には解けない
 }
}
