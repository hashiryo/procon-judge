#pragma once
#include "../common.hpp"
// U の基底を簡約した階段の形 (要の bit はその行にだけ立つ) に直し、V の元を U で割った余り r(v) を、U の要ごとに 1 回ずつの
// 消去で求める (簡約した形なので、要を見る順は問わない)。余りの写像 v -> r(v) の核が共通部分なので、(r(v) << 30) | v を
// 上 30 bit で掃き出し、上 30 bit が消えた元の下 30 bit を出す。
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 u32 pu[30];
 u32 P= 0;  // U の要の bit の集合
 for(int i= 0; i < n; ++i) {
  u32 x= u[i];
  for(u32 s= P & x; s; s&= s - 1) {  // 簡約した形なので、x の要の bit を 1 つずつ消せばよい
   const int b= __builtin_ctz(s);
   if(x >> b & 1) x^= pu[b];
  }
  const int b= 31 - __builtin_clz(x);
  for(u32 s= P; s; s&= s - 1) {  // 新しい要の bit を、ほかの行から消す
   const int c= __builtin_ctz(s);
   if(pu[c] >> b & 1) pu[c]^= x;
  }
  pu[b]= x, P|= 1u << b;
 }
 u64 piv[64]= {};
 int k= 0;
 for(int j= 0; j < m; ++j) {
  u32 r= v[j];
  for(u32 s= P & r; s; s&= s - 1) {
   const int b= __builtin_ctz(s);
   if(r >> b & 1) r^= pu[b];
  }
  u64 y= (u64(r) << 30) | v[j];
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
