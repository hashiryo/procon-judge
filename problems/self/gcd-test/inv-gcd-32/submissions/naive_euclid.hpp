#pragma once
#include "_shared/modulo-test/_common.hpp"
// 教科書どおりの拡張 Euclid の互除法 (参照実装)。32 bit の割り算を使い、係数は符号が交互になることを使って絶対値を u32 で持つ。
inline std::pair<u32, u32> run(u32 a, u32 b) {
 u32 r0= b, r1= a % b, s0= 0, s1= 1;
 bool pos= false;  // s0 の符号が正か (s_{-1} = 0 から始まる)
 while(r1) {
  u32 q= r0 / r1, t= r0 - q * r1;
  r0= r1, r1= t;
  t= s0 + q * s1;
  s0= s1, s1= t;
  pos= !pos;
 }
 u32 m= b / r0;
 return {r0, pos || s0 == 0 ? s0 : m - s0};
}
