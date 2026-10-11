#pragma once
#include "../common.hpp"
// Zassenhaus の方法。U の元を (u, 0)、V の元を (v, v) として 60 bit に並べ ((x << 30) | t)、上 30 bit の x で掃き出す。
// V の元が x = 0 まで消えたら、残った t は v + (V の元の和) で、それは U の元の和に等しいので、共通部分に入る。
// 要は bit ごとの表に持ち、上の bit から 1 bit ずつ見る。
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 u64 piv[60]= {};
 int k= 0;
 for(int i= 0; i < n + m; ++i) {
  u64 y= i < n ? u64(u[i]) << 30 : (u64(v[i - n]) << 30) | v[i - n];
  bool ins= false;
  for(int b= 59; b >= 30; --b) {
   if(!(y >> b & 1)) continue;
   if(piv[b]) y^= piv[b];
   else {
    piv[b]= y, ins= true;
    break;
   }
  }
  if(!ins) w[k++]= u32(y);
 }
 return k;
}
