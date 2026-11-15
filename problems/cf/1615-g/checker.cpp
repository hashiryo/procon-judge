// cf-1615-g のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
//
// 提出の出力が n 個の 1 以上 n 以下の整数で、入力の 0 でない要素を変えていないかを確かめ、その配列の値 (隣り合う 2 つが等しい
// k の種類の数) を、期待出力の配列の値と比べる。期待出力は参照実装の作った最適な配列なので、値が等しければ AC。提出の値の
// ほうが大きければ、参照実装の誤りとして WA にする (気づけるように)。
#include <cstdio>
#include <string>
#include <vector>
bool next_token(FILE* f, std::string& s) {
 s.clear();
 int c;
 while((c= fgetc(f)) != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));
 for(; c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t'; c= fgetc(f)) s+= char(c);
 return !s.empty();
}
bool parse_int(const std::string& s, long long& v) {
 if(s.empty() || s.size() > 12) return false;
 size_t i= s[0] == '-' ? 1 : 0;
 if(i == s.size()) return false;
 v= 0;
 for(; i < s.size(); ++i) {
  if(s[i] < '0' || s[i] > '9') return false;
  v= v * 10 + (s[i] - '0');
 }
 if(s[0] == '-') v= -v;
 return true;
}
// 配列を読んで確かめ、値を返す。形が違えば -1 と理由。
long long read_array(FILE* f, const std::vector<int>& a, std::string& why) {
 const int n= a.size();
 std::vector<int> b(n);
 std::string s;
 for(int i= 0; i < n; ++i) {
  long long v;
  if(!next_token(f, s)) return why= "要素が " + std::to_string(i) + " 個しかない", -1;
  if(!parse_int(s, v) || v < 1 || v > n) return why= std::to_string(i + 1) + " 番目の要素 " + s + " が 1 以上 n 以下でない", -1;
  if(a[i] != 0 && a[i] != v) return why= std::to_string(i + 1) + " 番目の要素を " + std::to_string(a[i]) + " から " + s + " に変えた", -1;
  b[i]= v;
 }
 if(next_token(f, s)) return why= "余分な出力 " + s, -1;
 std::vector<char> seen(n + 1, 0);
 long long val= 0;
 for(int i= 0; i + 1 < n; ++i)
  if(b[i] == b[i + 1] && !seen[b[i]]) seen[b[i]]= 1, ++val;
 return val;
}
int main(int argc, char** argv) {
 if(argc < 4) return fprintf(stderr, "usage: checker input output expected\n"), 2;
 FILE *fin= fopen(argv[1], "r"), *fout= fopen(argv[2], "r"), *fexp= fopen(argv[3], "r");
 if(!fin || !fout || !fexp) return fprintf(stderr, "ファイルを開けない\n"), 2;
 int n;
 if(fscanf(fin, "%d", &n) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
 std::vector<int> a(n);
 for(int& x: a)
  if(fscanf(fin, "%d", &x) != 1) return fprintf(stderr, "入力を読めない\n"), 2;
 std::string why;
 const long long got= read_array(fout, a, why);
 if(got < 0) return fprintf(stderr, "%s\n", why.c_str()), 1;
 const long long want= read_array(fexp, a, why);
 if(want < 0) return fprintf(stderr, "期待出力が正しい形でない: %s\n", why.c_str()), 2;
 if(got != want) return fprintf(stderr, "値が %lld (期待は %lld)%s\n", got, want, got > want ? "。参照実装の誤りかもしれない" : ""), 1;
 return 0;
}
