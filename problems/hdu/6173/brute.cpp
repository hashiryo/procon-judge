// hdu-6173 の愚直解。盤の各マスが矩形のどれかに入るかを数えて表のマスを決め、そのマスの lowbit(x) ⊗ lowbit(y) の xor をとる。
// 掃き出しもセグメント木も GF2p64 も使わないので、提出とは別の考え方になる。N <= 64 の入力でだけ使う (gen.py の seed 1000 以上)。
// nimber の積は、上下に分けて X = 2^(w/2) について X^2 = X + X/2 を使う形で計算する (8 bit どうしは表を引く)。
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
 int T;
 if(scanf("%d", &T) != 1) return 1;
 while(T--) {
  long long N;
  int M;
  if(scanf("%lld %d", &N, &M) != 2 || N > 64) return 1;
  std::vector<std::vector<char>> head(N + 1, std::vector<char>(N + 1));
  for(int i= 0; i < M; ++i) {
   int x1, y1, x2, y2;
   if(scanf("%d %d %d %d", &x1, &y1, &x2, &y2) != 4) return 1;
   for(int x= x1; x <= x2; ++x)
    for(int y= y1; y <= y2; ++y) head[x][y]= 1;
  }
  u64 sg= 0;
  for(int x= 1; x <= N; ++x)
   for(int y= 1; y <= N; ++y)
    if(head[x][y]) sg^= mul(u64(x & -x), u64(y & -y));
  puts(sg ? "Yong Chol" : "Brother");
 }
}
