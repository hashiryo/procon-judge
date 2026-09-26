#pragma once
#include "_shared/modulo-test/_common.hpp"
// 教科書的 Euclidean: a, b → a%b の繰り返し。
inline u64 run(u64 a, u64 b) {
 while (b) {
  u64 t= a % b;
  a= b;
  b= t;
 }
 return a;
}
