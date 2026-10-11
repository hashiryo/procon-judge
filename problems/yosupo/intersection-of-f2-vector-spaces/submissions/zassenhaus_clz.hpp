#pragma once
#include "../common.hpp"
// naive と同じ Zassenhaus の方法で、立っている最上位の bit を clz で飛ぶ。
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 u64 piv[64]= {};
 int k= 0;
 for(int i= 0; i < n; ++i) {  // U の元は線形独立なので、どれも要になる
  u64 y= u64(u[i]) << 30;
  for(;;) {
   const int b= 63 - __builtin_clzll(y);
   if(!piv[b]) {
    piv[b]= y;
    break;
   }
   y^= piv[b];
  }
 }
 for(int j= 0; j < m; ++j) {
  u64 y= (u64(v[j]) << 30) | v[j];
  for(;;) {
   if(y >> 30 == 0) {
    w[k++]= u32(y);
    break;
   }
   const int b= 63 - __builtin_clzll(y);
   if(!piv[b]) {
    piv[b]= y;
    break;
   }
   y^= piv[b];
  }
 }
 return k;
}
