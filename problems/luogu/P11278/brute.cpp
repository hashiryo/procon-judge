// luogu-P11278 の愚直解。配列をそのまま持ち、平方の操作では区間の要素を 1 つずつ nim 平方し、質問では区間を 1 つずつ足す。
// セグメント木も GF2p64 も使わないので、提出とは別の考え方になる。n と q が 2000 以下の入力でだけ使う (gen.py の seed 1000 以上)。
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
 int n, q;
 if(scanf("%d %d", &n, &q) != 2 || n > 2000 || q > 2000) return 1;
 std::vector<u64> a(n + 1);
 for(int i= 1; i <= n; ++i)
  if(scanf("%llu", &a[i]) != 1) return 1;
 while(q--) {
  int t, l, r;
  if(scanf("%d %d %d", &t, &l, &r) != 3) return 1;
  u64 x= 0, s= 0;
  for(int i= l; i <= r; ++i) {
   if(t == 1) a[i]= mul(a[i], a[i]);
   x^= a[i], s+= a[i];
  }
  if(t == 2) printf("%llu\n", x);
  if(t == 3) printf("%llu\n", s);
 }
}
