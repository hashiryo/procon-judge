// ソートの族 (self/sort/) の 64 bit の問題 (u64、i64) のハーネスが共有するもの。配列の作り方 (分布) と、並べた結果のハッシュ。
// 乱数は _common.hpp の splitmix64 を使う。_common.hpp を書き換えると self-sort-u32 のハーネスのキーが変わって全提出が
// 測り直しになるので、64 bit の分はこのヘッダに分けた。設計は algo-notes の notes/sort-problems.md。
#pragma once
#include "_shared/sort/_common.hpp"
namespace sort_family {
using i64= long long;
// 1 行の指定「分布 N seed 引数」から u64 の配列を作る。引数を使わない分布 (equal、sorted、reversed) でも 0 を書く。
//   range    [0, 引数] の一様分布 (引数が 2^64 - 1 なら全域)
//   few      ランダムに選んだ 引数 個の値から一様に選ぶ
//   equal    すべて同じ値
//   sorted   一様な u64 を昇順に並べたもの
//   reversed 一様な u64 を降順に並べたもの
//   nearly   sorted に 引数 回のランダムな入れ替えをしたもの
//   mask     一様な u64 と 引数 の bit ごとの and
inline std::vector<u64> make_u64(const char* dist, size_t n, u64 seed, u64 arg) {
 SplitMix64 rng(seed);
 std::vector<u64> a(n);
 if(!std::strcmp(dist, "range")) {
  if(arg == ~0ull)
   for(auto& x: a) x= rng();
  else
   for(auto& x: a) x= rng.below(arg + 1);
 } else if(!std::strcmp(dist, "few")) {
  std::vector<u64> vals(arg);
  for(auto& v: vals) v= rng();
  for(auto& x: a) x= vals[rng.below(arg)];
 } else if(!std::strcmp(dist, "equal")) {
  u64 v= rng();
  for(auto& x: a) x= v;
 } else if(!std::strcmp(dist, "sorted") || !std::strcmp(dist, "reversed") || !std::strcmp(dist, "nearly")) {
  for(auto& x: a) x= rng();
  if(!std::strcmp(dist, "reversed")) std::sort(a.begin(), a.end(), std::greater<u64>());
  else std::sort(a.begin(), a.end());
  if(!std::strcmp(dist, "nearly") && n > 0)
   for(u64 k= 0; k < arg; ++k) {
    size_t i= rng.below(n), j= rng.below(n);
    std::swap(a[i], a[j]);
   }
 } else if(!std::strcmp(dist, "mask")) {
  for(auto& x: a) x= rng() & arg;
 } else {
  std::fprintf(stderr, "unknown distribution: %s\n", dist);
  std::exit(1);
 }
 return a;
}
// 1 行の指定「分布 N seed 引数」から i64 の配列を作る。
//   range    [-引数, 引数] の一様分布 (引数は 2^63 - 1 以下)
//   nonneg   [0, 引数] の一様分布
//   few      ランダムに選んだ 引数 個の値 (全域) から一様に選ぶ
//   equal    すべて同じ値 (全域から 1 つ)
//   sorted   全域の一様な i64 を昇順に並べたもの
//   reversed 全域の一様な i64 を降順に並べたもの
//   nearly   sorted に 引数 回のランダムな入れ替えをしたもの
inline std::vector<i64> make_i64(const char* dist, size_t n, u64 seed, u64 arg) {
 SplitMix64 rng(seed);
 std::vector<i64> a(n);
 if(!std::strcmp(dist, "range")) {
  for(auto& x: a) x= i64(rng.below(2 * arg + 1) - arg);
 } else if(!std::strcmp(dist, "nonneg")) {
  for(auto& x: a) x= i64(rng.below(arg + 1));
 } else if(!std::strcmp(dist, "few")) {
  std::vector<i64> vals(arg);
  for(auto& v: vals) v= i64(rng());
  for(auto& x: a) x= vals[rng.below(arg)];
 } else if(!std::strcmp(dist, "equal")) {
  i64 v= i64(rng());
  for(auto& x: a) x= v;
 } else if(!std::strcmp(dist, "sorted") || !std::strcmp(dist, "reversed") || !std::strcmp(dist, "nearly")) {
  for(auto& x: a) x= i64(rng());
  if(!std::strcmp(dist, "reversed")) std::sort(a.begin(), a.end(), std::greater<i64>());
  else std::sort(a.begin(), a.end());
  if(!std::strcmp(dist, "nearly") && n > 0)
   for(u64 k= 0; k < arg; ++k) {
    size_t i= rng.below(n), j= rng.below(n);
    std::swap(a[i], a[j]);
   }
 } else {
  std::fprintf(stderr, "unknown distribution: %s\n", dist);
  std::exit(1);
 }
 return a;
}
// 並べた結果のハッシュ。値を splitmix64 の最後の混ぜる処理に通してから、先頭から h <- h K + x (mod 2^64) で畳む。
// 64 bit の値をそのまま足すと、差を割る 2 の冪の指数が 63 まで上がり、2 か所の入れ替えを必ず見つける性質が崩れるため。
inline u64 mix64(u64 z) {
 z= (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
 z= (z ^ (z >> 27)) * 0x94D049BB133111EBull;
 return z ^ (z >> 31);
}
template <class T> inline u64 hash_seq64(const std::vector<T>& a) {
 u64 h= 0;
 for(T x: a) h= h * 0x9E3779B97F4A7C15ull + mix64(u64(x));
 return h;
}
}  // namespace sort_family
