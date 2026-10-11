#pragma once
// bit ごとに最も右の元を残す xor 基底 (接頭辞の基底)。b[i] は最上位の bit が i の元 (無ければ 0)、p[i] はその元の位置。
// 新しい元を入れるとき、上の bit から見て、残っている元より新しい (位置が大きい) ほうを残し、古いほうを下へ流す。
// こうすると、位置が l 以上の元だけで、区間 [l, r) の元の張る空間の基底になる。
#include "../common.hpp"
struct PrefixBasis {
 u64 b[60];
 int p[60];
 void clear() {
  memset(b, 0, sizeof(b));
  memset(p, 0xff, sizeof(p));
 }
 void insert(u64 x, int pos) {
  while(x) {
   const int i= 63 - __builtin_clzll(x);
   if(!b[i]) {
    b[i]= x, p[i]= pos;
    return;
   }
   if(p[i] < pos) swap(b[i], x), swap(p[i], pos);
   x^= b[i];
  }
 }
 // 位置が l 以上の元だけで x を消せるか。
 bool contains(int l, u64 x) const {
  while(x) {
   const int i= 63 - __builtin_clzll(x);
   if(!b[i] || p[i] < l) return false;
   x^= b[i];
  }
  return true;
 }
};
