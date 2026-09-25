// GF2p64 の >> と << が、u64 の >> と << と同じに振る舞うかを確かめる。
// GF2p64 は中身の u64 が Library Checker の F_{2^64} の整数表現 (x^i の係数が i ビット目) そのものなので、
// u64 の入出力がそのまま正解になる。書いて読み戻すだけの往復では、>> と << が揃って同じ間違い方を
// したとき (16 進で書いて 16 進で読むなど) に通ってしまうので、u64 と突き合わせる。
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "mylib/algebra/GF2p64.hpp"
using namespace std;
using u64= unsigned long long;
int checks= 0, failures= 0;
void check(bool ok, const string& what) {
 ++checks;
 if(!ok) ++failures, cerr << "NG: " << what << '\n';
}
template <class T> string write(const T& x, ios_base& (*base)(ios_base&)) {
 ostringstream os;
 os << base << x;
 return os.str();
}
// 書き出し: 同じ値を GF2p64 と u64 で書いて、同じ文字列になる。10 進のほか、ストリームの基数の指定にも従う。
void test_write(u64 v) {
 for(auto base: {dec, hex}) {
  string g= write(GF2p64(v), base), u= write(v, base);
  check(g == u, "<< " + u + " が " + g + " になった");
 }
}
// 読み込み: 同じ文字列を GF2p64 と u64 で読めなくなるまで読んで、読めた値と最後の状態が同じになる。
void test_read(const string& text) {
 istringstream gs(text), us(text);
 vector<u64> gv, uv;
 GF2p64 g;
 u64 u;
 while(gs >> g) gv.push_back(u64(g));
 while(us >> u) uv.push_back(u);
 check(gv == uv, ">> で \"" + text + "\" から読んだ値が u64 と違う");
 check(gs.rdstate() == us.rdstate(), ">> で \"" + text + "\" を読んだあとの状態が u64 と違う");
}
// 往復: 書いたものを読み戻すと元の値になる。
void test_round_trip(u64 v) {
 stringstream ss;
 ss << GF2p64(v);
 GF2p64 g;
 ss >> g;
 check(bool(ss) && u64(g) == v, "往復で " + to_string(v) + " が " + to_string(u64(g)) + " になった");
}
signed main() {
 // 2^63 以上は、符号付きの整数で読み書きすると壊れる。
 vector<u64> values= {0, 1, 2, 27, (1ull << 32) - 1, 1ull << 32, (1ull << 63) - 1, 1ull << 63, (1ull << 63) + 1, 0xB00000000000001Bull, ~0ull - 1, ~0ull};
 mt19937_64 rng(20260925);
 for(int i= 0; i < 10000; ++i) values.push_back(rng());
 for(u64 v: values) test_write(v), test_read(to_string(v)), test_round_trip(v);
 // 区切りの空白の入れ方を変えても、u64 と同じに読める。
 string spaced, lines, mixed;
 for(int i= 0; i < 12; ++i) {
  spaced+= to_string(values[i]) + ' ';
  lines+= to_string(values[i]) + '\n';
  mixed+= string(i % 3 + 1, " \t\n"[i % 3]) + to_string(values[i]);
 }
 test_read(spaced), test_read(lines), test_read(mixed);
 // 読めない入力と、64 bit に収まらない入力。読めなくなった位置と状態も u64 と同じになる。
 for(string text: {"", " \n", "abc", "12abc", "12 abc 34", "+5", "18446744073709551615", "18446744073709551616", "0x10"}) test_read(text);
 // 続けて読む書き方と、続けて書く書き方。
 {
  istringstream is("3 18446744073709551615\n9223372036854775808");
  GF2p64 a, b, c;
  is >> a >> b >> c;
  check(bool(is) && u64(a) == 3 && u64(b) == ~0ull && u64(c) == 1ull << 63, "続けて読んだ値が違う");
  ostringstream os;
  os << a << ' ' << b << '\n' << c;
  check(os.str() == "3 18446744073709551615\n9223372036854775808", "続けて書いた文字列が違う");
 }
 if(failures) {
  cerr << checks << " 件のうち " << failures << " 件が違った\n";
  return 1;
 }
 return 0;
}
