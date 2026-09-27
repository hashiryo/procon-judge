// self-gf2-64-log-any のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 答えが 2^64-1 以外なら、それが 2^64-2 以下で a^k = b を満たすことを確かめる。2^64-1 (解なし) なら、
// 解が本当に無いことを確かめる。2^64-1 は平方因子を持たないので、ある素因数 p で
// a^((2^64-1)/p) = 1 かつ b^((2^64-1)/p) ≠ 1 となることが、解が無いことと同値になる。
//
// 期待出力は読まない。判定は入力と提出の出力だけで決まる。参照実装も提出の 1 本としてこの
// チェッカで判定されるので、参照実装が間違えればその行が WA になって見える。
//
// pj はチェッカを環境のフラグを付けずに組むので、pclmul などは使わず素朴に掛ける。
#include <cstdio>
#include <string>
using u64= unsigned long long;

constexpr u64 ORDER= ~0ull;  // 乗法群の位数 2^64-1。解なしの答えにも使う
constexpr u64 PRIMES[]= {3, 5, 17, 257, 641, 65537, 6700417};

// GF(2)[x] / (x^64 + x^4 + x^3 + x + 1) の積。4 bit ずつの窓で 128 bit の積を作ってから落とす。
u64 gf_mul(u64 a, u64 b) {
 u64 tl[16], th[16];
 tl[0]= th[0]= 0, tl[1]= a, th[1]= 0;
 for(int i= 2; i < 16; i+= 2) {
  tl[i]= tl[i / 2] << 1, th[i]= (th[i / 2] << 1) | (tl[i / 2] >> 63);
  tl[i + 1]= tl[i] ^ a, th[i + 1]= th[i];
 }
 u64 lo= 0, hi= 0;
 for(int s= 60; s >= 0; s-= 4) {
  hi= (hi << 4) | (lo >> 60), lo<<= 4;
  const unsigned nib= (b >> s) & 15;
  lo^= tl[nib], hi^= th[nib];
 }
 // hi·x^64 ≡ hi·(x^4 + x^3 + x + 1)。シフトではみ出た分 (4 bit 未満) をもう一度同じように落とす。
 const u64 over= (hi >> 63) ^ (hi >> 61) ^ (hi >> 60);
 return lo ^ hi ^ (hi << 1) ^ (hi << 3) ^ (hi << 4) ^ over ^ (over << 1) ^ (over << 3) ^ (over << 4);
}
u64 gf_pow(u64 a, u64 e) {
 u64 r= 1;
 for(; e; e>>= 1, a= gf_mul(a, a))
  if(e & 1) r= gf_mul(r, a);
 return r;
}
bool solvable(u64 a, u64 b) {
 for(u64 p: PRIMES)
  if(gf_pow(a, ORDER / p) == 1 && gf_pow(b, ORDER / p) != 1) return false;
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
 u64 x= 0;
 for(char c: s) {
  if(c < '0' || c > '9') return false;
  const u64 d= u64(c - '0');
  if(x > (ORDER - d) / 10) return false;
  x= x * 10 + d;
 }
 v= x;
 return true;
}

int main(int argc, char** argv) {
 if(argc < 4) {
  fprintf(stderr, "usage: checker input output answer\n");
  return 2;
 }
 FILE* in= fopen(argv[1], "r");
 FILE* out= fopen(argv[2], "r");
 if(!in || !out) {
  fprintf(stderr, "ファイルを開けません\n");
  return 2;
 }
 std::string tok;
 u64 t;
 if(!next_token(in, tok) || !parse_u64(tok, t)) {
  fprintf(stderr, "入力の件数が読めません\n");
  return 2;
 }
 for(u64 i= 0; i < t; ++i) {
  u64 a, b, k;
  if(!next_token(in, tok) || !parse_u64(tok, a) || !next_token(in, tok) || !parse_u64(tok, b) || !a || !b) {
   fprintf(stderr, "入力の %llu 組目が読めません (0 は入力に出ない約束)\n", i + 1);
   return 2;
  }
  if(!next_token(out, tok)) {
   fprintf(stderr, "出力が %llu 個しかありません (%llu 個要ります)\n", i, t);
   return 1;
  }
  if(!parse_u64(tok, k)) {
   fprintf(stderr, "%llu 個目の出力 '%s' が 0 以上 2^64-1 以下の整数として読めません\n", i + 1, tok.c_str());
   return 1;
  }
  if(k != ORDER) {
   if(gf_pow(a, k) != b) {
    fprintf(stderr, "%llu 個目: a = %llu, b = %llu に対して k = %llu は a^k = b を満たしません\n", i + 1, a, b, k);
    return 1;
   }
  } else if(solvable(a, b)) {
   fprintf(stderr, "%llu 個目: a = %llu, b = %llu には解があるのに、解なし (2^64-1) と答えました\n", i + 1, a, b);
   return 1;
  }
 }
 if(next_token(out, tok)) {
  fprintf(stderr, "出力が %llu 個より多くあります\n", t);
  return 1;
 }
 return 0;
}
