// cf-1310-f のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の答えが x なら a^x = b (nimber の冪) を確かめ、-1 なら解が本当に無いことを確かめる。乗法群の位数 2^64-1 は
// 平方因子を持たないので、ある素因数 p で a^((2^64-1)/p) = 1 かつ b^((2^64-1)/p) ≠ 1 となることが、解が無いことと同値になる。
// 期待出力は読まない。判定は入力と提出の出力だけで決まる。
//
// nimber の積は GF2p64 を使わず、上下に分けて X = 2^(w/2) について X^2 = X + X/2 を使う形で計算する (8 bit どうしは表を引く)。
#include <cstdio>
#include <string>
using u64= unsigned long long;

constexpr u64 ORDER= ~0ull;  // 乗法群の位数 2^64-1
constexpr u64 PRIMES[]= {3, 5, 17, 257, 641, 65537, 6700417};

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
u64 power(u64 a, u64 e) {
 u64 r= 1;
 for(; e; e>>= 1, a= mul(a, a))
  if(e & 1) r= mul(r, a);
 return r;
}
bool solvable(u64 a, u64 b) {
 for(u64 p: PRIMES)
  if(power(a, ORDER / p) == 1 && power(b, ORDER / p) != 1) return false;
 return true;
}

bool next_token(FILE* f, std::string& s) {
 s.clear();
 int c;
 while((c= fgetc(f)) != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));
 for(; c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t'; c= fgetc(f)) s+= char(c);
 return !s.empty();
}
bool parse_u64(const std::string& s, u64& v) {
 if(s.empty() || s.size() > 20) return false;
 v= 0;
 for(char c: s) {
  if(c < '0' || c > '9') return false;
  const u64 d= u64(c - '0');
  if(v > (~0ull - d) / 10) return false;
  v= v * 10 + d;
 }
 return true;
}

int main(int argc, char** argv) {
 if(argc < 3) return 2;
 for(int a= 0; a < 256; ++a)
  for(int b= 0; b < 256; ++b) T8[a][b]= (unsigned char)mul_rec(a, b, 8);
 if(mul(4, 4) != 6 || mul(8, 8) != 13 || mul(32, 64) != 141 || mul(5, 6) != 8) return 2;
 FILE* in= fopen(argv[1], "r");
 FILE* out= fopen(argv[2], "r");
 if(!in || !out) return 2;
 int t;
 if(fscanf(in, "%d", &t) != 1) return 2;
 std::string tok;
 for(int i= 1; i <= t; ++i) {
  u64 a, b, x;
  if(fscanf(in, "%llu %llu", &a, &b) != 2) return 2;
  if(!next_token(out, tok)) return fprintf(stderr, "%d 個目の答えがありません\n", i), 1;
  if(tok == "-1") {
   if(solvable(a, b)) return fprintf(stderr, "%d 個目 (a = %llu, b = %llu): -1 を出しましたが、解があります\n", i, a, b), 1;
   continue;
  }
  if(!parse_u64(tok, x)) return fprintf(stderr, "%d 個目の答え %s は 0 以上 2^64 未満の整数でも -1 でもありません\n", i, tok.c_str()), 1;
  if(power(a, x) != b) return fprintf(stderr, "%d 個目 (a = %llu, b = %llu): a^%llu = %llu で、b と違います\n", i, a, b, x, power(a, x)), 1;
 }
 if(next_token(out, tok)) return fprintf(stderr, "t 個の答えのあとに余計な出力 %s があります\n", tok.c_str()), 1;
 return 0;
}
