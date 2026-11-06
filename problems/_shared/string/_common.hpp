// 文字列の基本の道具の族 (self/string/) のハーネスが共有するもの。文字列の作り方 (族) と乱数と、作り方の 1 行の読み方。
// 入力ファイルには文字列の作り方だけを書き、文字列はハーネスが計測区間の外で作る。10^6 文字の文字列とクエリを入力ファイルに
// 書くと、CI で作って読むだけで時間がかかるため (self/sort/ と同じ形)。設計は algo-notes の notes/string-basics.md。
// 乱数は splitmix64。std::uniform_int_distribution は標準ライブラリによって結果が違うので使わない。
#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
namespace string_family {
using u64= unsigned long long;
struct SplitMix64 {
 u64 x;
 explicit SplitMix64(u64 seed): x(seed) {}
 u64 operator()() {
  u64 z= (x+= 0x9E3779B97F4A7C15ull);
  z= (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z= (z ^ (z >> 27)) * 0x94D049BB133111EBull;
  return z ^ (z >> 31);
 }
 // [0, n) の一様乱数 (n >= 1)。
 u64 below(u64 n) { return u64(((unsigned __int128)(*this)() * n) >> 64); }
};
// 1 行の指定「族 n seed x y z」から英小文字の文字列を作る。使わない引数にも 0 を書く。
//   random  x 種類の文字 (a から x 個) の一様分布。1 <= x <= 26
//   period  y 種類の文字の一様な長さ x の塊を繰り返したもの
//   noisy   period と同じ文字列の、一様に選んだ z か所を y 種類の文字の一様な 1 文字に置き換えたもの
//   fib     フィボナッチ文字列 (a、ab、aba、abaab、... と前の 2 つを繋いだもの) の先頭 n 文字
//   thue    Thue–Morse 列 (位置 i の文字は i の 2 進の 1 の数が偶数なら a、奇数なら b) の先頭 n 文字
//   carry   位置 i の文字が a + (i + 1 の 2 進の末尾の 0 の数) の列 (abacabadabacaba...)
//   blocks  a を x - 1 個と b を 1 個並べた長さ x の塊を繰り返したもの
inline std::string make_string(const char* fam, size_t n, u64 seed, u64 x, u64 y, u64 z) {
 SplitMix64 rng(seed);
 std::string s(n, 'a');
 auto bad= [&](const char* what) {
  std::fprintf(stderr, "bad string spec %s: %s\n", fam, what);
  std::exit(1);
 };
 if(!std::strcmp(fam, "random")) {
  if(x < 1 || x > 26) bad("x");
  for(auto& c: s) c= char('a' + rng.below(x));
 } else if(!std::strcmp(fam, "period") || !std::strcmp(fam, "noisy")) {
  if(x < 1 || y < 1 || y > 26) bad("x, y");
  std::string blk(x, 'a');
  for(auto& c: blk) c= char('a' + rng.below(y));
  for(size_t i= 0; i < n; ++i) s[i]= blk[i % x];
  if(!std::strcmp(fam, "noisy") && n > 0)
   for(u64 k= 0; k < z; ++k) s[rng.below(n)]= char('a' + rng.below(y));
 } else if(!std::strcmp(fam, "fib")) {
  std::string a= "a", b= "ab";
  while(b.size() < n) {
   std::string c= b + a;
   a.swap(b), b.swap(c);
  }
  s= b.substr(0, n);
 } else if(!std::strcmp(fam, "thue")) {
  for(size_t i= 0; i < n; ++i) s[i]= char('a' + (__builtin_popcountll(i) & 1));
 } else if(!std::strcmp(fam, "carry")) {
  for(size_t i= 0; i < n; ++i) s[i]= char('a' + __builtin_ctzll(i + 1));
 } else if(!std::strcmp(fam, "blocks")) {
  if(x < 1) bad("x");
  for(size_t i= 0; i < n; ++i) s[i]= i % x == x - 1 ? 'b' : 'a';
 } else bad("unknown family");
 return s;
}
// 標準入力から「族 n seed x y z」の 1 行を読んで文字列を作る。
inline std::string read_string() {
 char fam[16];
 unsigned long long n, seed, x, y, z;
 if(std::scanf("%15s %llu %llu %llu %llu %llu", fam, &n, &seed, &x, &y, &z) != 6) {
  std::fprintf(stderr, "input format error\n");
  std::exit(1);
 }
 return make_string(fam, size_t(n), seed, x, y, z);
}
}  // namespace string_family
