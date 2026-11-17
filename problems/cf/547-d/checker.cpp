// cf-547-d のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の出力が n 文字の r と b からなる 1 つの文字列で、どの縦の線 (x が同じ点) と横の線 (y が同じ点) でも r と b の数の差が
// 1 以下であることを確かめる。期待出力は読まない。
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
bool next_token(FILE* f, std::string& s) {
 s.clear();
 int c;
 while((c= fgetc(f)) != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));
 for(; c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t'; c= fgetc(f)) s+= char(c);
 return !s.empty();
}
int main(int argc, char** argv) {
 if(argc < 3) return fprintf(stderr, "usage: checker input output [expected]\n"), 2;
 FILE *fin= fopen(argv[1], "r"), *fout= fopen(argv[2], "r");
 if(!fin || !fout) return fprintf(stderr, "ファイルを開けない\n"), 2;
 int n;
 if(fscanf(fin, "%d", &n) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
 std::vector<int> x(n), y(n);
 for(int i= 0; i < n; ++i)
  if(fscanf(fin, "%d %d", &x[i], &y[i]) != 2) return fprintf(stderr, "入力を読めない\n"), 2;
 std::string s, extra;
 if(!next_token(fout, s)) return fprintf(stderr, "出力が空\n"), 1;
 if(next_token(fout, extra)) return fprintf(stderr, "余分な出力 %s\n", extra.c_str()), 1;
 if((int)s.size() != n) return fprintf(stderr, "長さが %zu (期待は %d)\n", s.size(), n), 1;
 std::map<int, int> dx, dy;
 for(int i= 0; i < n; ++i) {
  if(s[i] != 'r' && s[i] != 'b') return fprintf(stderr, "%d 文字目が r でも b でもない %c\n", i + 1, s[i]), 1;
  const int d= s[i] == 'r' ? 1 : -1;
  dx[x[i]]+= d, dy[y[i]]+= d;
 }
 for(auto& [k, v]: dx)
  if(abs(v) > 1) return fprintf(stderr, "縦の線 x = %d で r と b の差が %d\n", k, v), 1;
 for(auto& [k, v]: dy)
  if(abs(v) > 1) return fprintf(stderr, "横の線 y = %d で r と b の差が %d\n", k, v), 1;
 return 0;
}
