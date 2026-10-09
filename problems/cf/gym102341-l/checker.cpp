// cf-gym102341-l のチェッカ。引数は入力、提出の出力、期待出力の順 (pj の約束)。
// 先手の勝ちは First と Latias のどちらでも、後手の勝ちは Second と Latios のどちらでもよいので、勝つ側だけを比べる。
#include <cstdio>
#include <string>

bool next_token(FILE* f, std::string& s) {
 s.clear();
 int c;
 while((c= fgetc(f)) != EOF && (c == ' ' || c == '\n' || c == '\r' || c == '\t'));
 for(; c != EOF && c != ' ' && c != '\n' && c != '\r' && c != '\t'; c= fgetc(f)) s+= char(c);
 return !s.empty();
}
// 先手の勝ちなら 1、後手の勝ちなら 0、どちらでもない語なら -1
int side(const std::string& s) {
 if(s == "First" || s == "Latias") return 1;
 if(s == "Second" || s == "Latios") return 0;
 return -1;
}

int main(int argc, char** argv) {
 if(argc < 4) return 2;
 FILE* out= fopen(argv[2], "r");
 FILE* exp= fopen(argv[3], "r");
 if(!out || !exp) return 2;
 std::string o, e, extra;
 if(!next_token(exp, e) || side(e) < 0) return 2;
 if(!next_token(out, o)) return fprintf(stderr, "出力が空です\n"), 1;
 if(side(o) < 0) return fprintf(stderr, "%s は First、Latias、Second、Latios のどれでもありません\n", o.c_str()), 1;
 if(next_token(out, extra)) return fprintf(stderr, "余計な出力 %s があります\n", extra.c_str()), 1;
 return side(o) == side(e) ? 0 : (fprintf(stderr, "勝つ側が違います (期待は %s)\n", e.c_str()), 1);
}
