// cf-gym102341-l の愚直解。全部の置換について M_{i,σ(i)} の nim 積をとり、その xor (パーマネント) が 0 でなければ先手の勝ち。
// 行列式も掃き出しも GF2p64 も使わないので、提出とは別の考え方になる。n <= 8 の入力でだけ使う (gen.py の seed 1000 以上)。
// nimber の積は、上下に分けて X = 2^(w/2) について X^2 = X + X/2 を使う形で計算する (8 bit どうしは表を引く)。
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
 int n;
 if(scanf("%d", &n) != 1 || n > 8) return 1;
 std::vector<std::vector<u64>> m(n, std::vector<u64>(n));
 for(auto& row: m)
  for(auto& v: row)
   if(scanf("%llu", &v) != 1) return 1;
 std::vector<int> p(n);
 for(int i= 0; i < n; ++i) p[i]= i;
 u64 per= 0;
 do {
  u64 prod= 1;
  for(int i= 0; i < n && prod; ++i) prod= mul(prod, m[i][p[i]]);
  per^= prod;
 } while(std::next_permutation(p.begin(), p.end()));
 puts(per ? "First" : "Second");
}
