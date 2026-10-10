#pragma once
#include "_shared/modulo-test/_common.hpp"
// 教科書どおりの拡張 Euclid の互除法 (参照実装)。a の係数 s_i の符号は (-1)^i で交互になるので、
// 絶対値を u64 で持って |s_{i+1}| = |s_{i-1}| + q_i |s_i| と足していく。絶対値は b / g 以下なので u64 全体の入力で溢れない。
inline std::pair<u64, u64> run(u64 a, u64 b) {
 u64 r0= b, r1= a % b, s0= 0, s1= 1;
 bool pos= false;  // s0 の符号が正か (s_{-1} = 0 から始まる)
 while(r1) {
  u64 q= r0 / r1, t= r0 - q * r1;
  r0= r1, r1= t;
  t= s0 + q * s1;
  s0= s1, s1= t;
  pos= !pos;
 }
 u64 m= b / r0;
 return {r0, pos || s0 == 0 ? s0 : m - s0};
}
