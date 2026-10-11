#pragma once
// abc223-h の愚直解 (提出ではない)。問い合わせのたびに、区間 a[l..r) の元を上の bit から掃き出して基底を作り、x を消せる
// かを見る。接頭辞の基底も位置も使わないので、提出とは別の考え方になる。小さい入力でだけ使う
// (pj testdata crosscheck --brute brute.hpp)。
#include "common.hpp"
struct Solver {
 const vector<u64>& a;
 explicit Solver(const vector<u64>& a): a(a) {}
 bool query(int l, int r, u64 x) const {
  u64 b[64]= {};
  for(int i= l; i < r; ++i) {
   u64 y= a[i];
   for(int k= 63; k >= 0; --k)
    if(y >> k & 1) {
     if(!b[k]) {
      b[k]= y;
      break;
     }
     y^= b[k];
    }
  }
  for(int k= 63; k >= 0; --k)
   if(x >> k & 1) {
    if(!b[k]) return false;
    x^= b[k];
   }
  return true;
 }
};
