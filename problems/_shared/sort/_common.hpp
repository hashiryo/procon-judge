// ソートの族 (self/sort/) のハーネスが共有するもの。配列の作り方 (分布) と、並べた結果のハッシュ。
// 入力ファイルには配列の作り方だけを書き、配列はハーネスが計測区間の外で作る。10^6 個の数を入力ファイルに
// 書くと 1 ケースが 10 MB になり、CI で作って読むだけで時間がかかるため。設計は algo-notes の notes/sort-problems.md。
// 乱数は splitmix64。std::uniform_int_distribution は標準ライブラリによって結果が違うので使わない。
#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vector>
namespace sort_family {
using u32= unsigned int;
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
 u32 next_u32() { return u32((*this)() >> 32); }
 // [0, n) の一様乱数 (n >= 1)。上位 64 bit を取る掛け算なので、偏りは 2^-64 n 程度で無視できる。
 u64 below(u64 n) { return u64(((unsigned __int128)(*this)() * n) >> 64); }
};
// 1 行の指定「分布 N seed 引数」から u32 の配列を作る。引数を使わない分布 (equal、sorted、reversed) でも 0 を書く。
//   range    [0, 引数] の一様分布
//   few      ランダムに選んだ 引数 個の値から一様に選ぶ
//   equal    すべて同じ値
//   sorted   一様な u32 を昇順に並べたもの
//   reversed 一様な u32 を降順に並べたもの
//   nearly   sorted に 引数 回のランダムな入れ替えをしたもの
//   mask     一様な u32 と 引数 の bit ごとの and
inline std::vector<u32> make_u32(const char* dist, size_t n, u64 seed, u64 arg) {
 SplitMix64 rng(seed);
 std::vector<u32> a(n);
 if(!std::strcmp(dist, "range")) {
  for(auto& x: a) x= u32(rng.below(arg + 1));
 } else if(!std::strcmp(dist, "few")) {
  std::vector<u32> vals(arg);
  for(auto& v: vals) v= rng.next_u32();
  for(auto& x: a) x= vals[rng.below(arg)];
 } else if(!std::strcmp(dist, "equal")) {
  u32 v= rng.next_u32();
  for(auto& x: a) x= v;
 } else if(!std::strcmp(dist, "sorted") || !std::strcmp(dist, "reversed") || !std::strcmp(dist, "nearly")) {
  for(auto& x: a) x= rng.next_u32();
  if(!std::strcmp(dist, "reversed")) std::sort(a.begin(), a.end(), std::greater<u32>());
  else std::sort(a.begin(), a.end());
  if(!std::strcmp(dist, "nearly") && n > 0)
   for(u64 k= 0; k < arg; ++k) {
    size_t i= rng.below(n), j= rng.below(n);
    std::swap(a[i], a[j]);
   }
 } else if(!std::strcmp(dist, "mask")) {
  for(auto& x: a) x= rng.next_u32() & u32(arg);
 } else {
  std::fprintf(stderr, "unknown distribution: %s\n", dist);
  std::exit(1);
 }
 return a;
}
// 並べた結果のハッシュ。先頭から h <- h K + x (mod 2^64) で畳む。K = 5 (mod 8) なので、N < 2^20 なら
// 2 か所を入れ替えただけの誤りでも必ず値が変わる (差の 2 の冪の指数が 31 + 2 + 19 以下で 64 に届かない)。
inline u64 hash_seq(const std::vector<u32>& a) {
 u64 h= 0;
 for(u32 x: a) h= h * 0x9E3779B97F4A7C15ull + x;
 return h;
}
}  // namespace sort_family
