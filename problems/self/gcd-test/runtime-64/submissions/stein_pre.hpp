#pragma once
#include "_shared/modulo-test/_common.hpp"
// purplesyringa の「Faster practical modular inversion」の gcd の書き方。シフトをループの頭に置き、
// 次に引く差の末尾の 0 を入れ替えより先に数える。鎖の段数は Library の binary_gcd と同じ見込みで、比べるための 1 本。
inline u64 run(u64 a, u64 b) {
 if(a == 0 || b == 0) return a | b;
 int z= __builtin_ctzll(a | b), q= __builtin_ctzll(a);
 b>>= __builtin_ctzll(b);
 while(a != 0) {
  a>>= q;
  u64 d= a - b;
  q= std::countr_zero(d);  // d = 0 のときは a も 0 になって抜けるので、q = 64 は使われない
  bool f= a < b;
  u64 na= f ? b - a : d;
  b= f ? a : b, a= na;
 }
 return b << z;
}
