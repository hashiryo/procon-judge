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
 // contains と同じ判定を、分岐なしで書く。位置が l より前の欄は 0 とみなし、上の bit から x = min(x, x ^ b) を当てる。
 // b の最上位の bit が x に立っているときだけ x が小さくなり、欄が 0 なら x は変わらない。最後に x が 0 なら作れる。
 bool contains_min(int l, u64 x) const {
  for(int i= 59; i >= 0; --i) {
   const u64 m= p[i] >= l ? b[i] : 0;
   x= min(x, x ^ m);
  }
  return x == 0;
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
