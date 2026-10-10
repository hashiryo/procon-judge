#pragma once
#include "_shared/modulo-test/_common.hpp"
// 教科書どおりの Euclid の互除法 (参照実装)。32 bit の割り算を使う。
inline u32 run(u32 a, u32 b) {
 while(b) {
  u32 t= a % b;
  a= b, b= t;
 }
 return a;
}
