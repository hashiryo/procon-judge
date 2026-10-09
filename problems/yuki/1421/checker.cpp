// yuki-1421 のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出が N 個の値を出したら、どれも 0 以上 2^30 未満で、旅人の報告をすべて満たすことを確かめる。
// -1 を出したら、報告に本当に矛盾があることを、町の集合を 50 bit の mask にした xor の掃き出しで確かめる。
// 期待出力は読まない。判定は入力と提出の出力だけで決まる。
#include <cstdio>
#include <string>
#include <vector>
using u64= unsigned long long;

bool next_token(FILE* f, std::string& s) {
 s.clear();
 int c;
 while((c= fgetc(f)) != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));
 for(; c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t'; c= fgetc(f)) s+= char(c);
 return !s.empty();
}
bool parse_value(const std::string& s, u64& v) {
 if(s.empty() || s.size() > 10) return false;
 v= 0;
 for(char c: s) {
  if(c < '0' || c > '9') return false;
  v= v * 10 + u64(c - '0');
 }
 return v < (u64(1) << 30);
}
int main(int argc, char** argv) {
 if(argc < 3) return 2;
 FILE* in= fopen(argv[1], "r");
 FILE* out= fopen(argv[2], "r");
 if(!in || !out) return 2;
 int N, M;
 if(fscanf(in, "%d %d", &N, &M) != 2) return 2;
 std::vector<u64> mask(M), y(M);
 for(int j= 0; j < M; ++j) {
  int A;
  if(fscanf(in, "%d", &A) != 1) return 2;
  for(int k= 0; k < A; ++k) {
   int b;
   if(fscanf(in, "%d", &b) != 1) return 2;
   mask[j]|= u64(1) << (b - 1);
  }
  if(fscanf(in, "%llu", &y[j]) != 1) return 2;
 }
 std::string tok;
 if(!next_token(out, tok)) return fprintf(stderr, "出力が空です\n"), 1;
 if(tok == "-1") {
  if(next_token(out, tok)) return fprintf(stderr, "-1 のあとに余計な出力 %s があります\n", tok.c_str()), 1;
  // 町の集合の mask の xor の掃き出し。mask が 0 になって値が残れば矛盾。
  u64 bm[64]= {}, bv[64]= {};
  for(int j= 0; j < M; ++j) {
   u64 m= mask[j], v= y[j];
   bool added= false;
   for(int i= 63; i >= 0 && m; --i)
    if(m >> i & 1) {
     if(!bm[i]) {
      bm[i]= m, bv[i]= v, added= true;
      break;
     }
     m^= bm[i], v^= bv[i];
    }
   if(!added && v) return 0;
  }
  return fprintf(stderr, "-1 を出しましたが、報告に矛盾がありません\n"), 1;
 }
 std::vector<u64> x(N);
 for(int i= 0; i < N; ++i) {
  if(i && !next_token(out, tok)) return fprintf(stderr, "値が %d 個しかありません (N = %d)\n", i, N), 1;
  if(!parse_value(tok, x[i])) return fprintf(stderr, "X_%d = %s は 0 以上 2^30 未満の整数ではありません\n", i + 1, tok.c_str()), 1;
 }
 if(next_token(out, tok)) return fprintf(stderr, "N 個の値のあとに余計な出力 %s があります\n", tok.c_str()), 1;
 for(int j= 0; j < M; ++j) {
  u64 s= 0;
  for(int i= 0; i < N; ++i)
   if(mask[j] >> i & 1) s^= x[i];
  if(s != y[j]) return fprintf(stderr, "旅人 %d の報告 (Y = %llu) と合いません (xor = %llu)\n", j + 1, y[j], s), 1;
 }
 return 0;
}
