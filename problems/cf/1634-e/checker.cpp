// cf-1634-e のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出が YES なら、続く m 個の文字列が配列ごとの長さで L と R だけからなり、どの配列もちょうど半分が L で、どの値も L に入る回数と
// R に入る回数が等しいことを確かめる。NO なら、分け方が本当に無いこと (全体で奇数回出てくる値があること) を確かめる。どの値も
// 偶数回なら、配列と値を頂点、要素を辺にした二部グラフの Euler 分割で分け方が作れるので、これは分け方が無いことと同値になる。
// YES と NO は大文字と小文字を区別しない。期待出力は読まない。
#include <cctype>
#include <cstdio>
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
 int m;
 if(fscanf(fin, "%d", &m) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
 std::vector<std::vector<int>> a(m);
 std::map<int, int> total;
 for(auto& v: a) {
  int n;
  if(fscanf(fin, "%d", &n) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
  v.resize(n);
  for(int& x: v) {
   if(fscanf(fin, "%d", &x) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
   ++total[x];
  }
 }
 bool possible= true;
 for(auto& [v, c]: total) possible&= c % 2 == 0;
 std::string s;
 if(!next_token(fout, s)) return fprintf(stderr, "出力が空\n"), 1;
 for(char& c: s) c= char(toupper((unsigned char)c));
 if(s == "NO") {
  if(possible) return fprintf(stderr, "NO と答えたが、どの値も偶数回なので分け方がある\n"), 1;
  if(next_token(fout, s)) return fprintf(stderr, "余分な出力 %s\n", s.c_str()), 1;
  return 0;
 }
 if(s != "YES") return fprintf(stderr, "YES でも NO でもない %s\n", s.c_str()), 1;
 std::map<int, int> d;
 for(int i= 0; i < m; ++i) {
  if(!next_token(fout, s)) return fprintf(stderr, "%d 番目の配列の文字列が無い\n", i + 1), 1;
  if(s.size() != a[i].size()) return fprintf(stderr, "%d 番目の配列の文字列の長さが %zu (期待は %zu)\n", i + 1, s.size(), a[i].size()), 1;
  int l= 0;
  for(size_t j= 0; j < s.size(); ++j) {
   if(s[j] != 'L' && s[j] != 'R') return fprintf(stderr, "%d 番目の配列の文字列に L と R 以外の文字 %c\n", i + 1, s[j]), 1;
   l+= s[j] == 'L';
   d[a[i][j]]+= s[j] == 'L' ? 1 : -1;
  }
  if(2 * l != (int)s.size()) return fprintf(stderr, "%d 番目の配列の L が %d 個で、半分でない\n", i + 1, l), 1;
 }
 if(next_token(fout, s)) return fprintf(stderr, "余分な出力 %s\n", s.c_str()), 1;
 for(auto& [v, c]: d)
  if(c != 0) return fprintf(stderr, "値 %d の L と R の回数の差が %d\n", v, c), 1;
 return 0;
}
