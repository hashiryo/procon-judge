#pragma once
#include "../common.hpp"
// Zassenhaus の方法を、分岐なしの消去で書く。Library Checker の首位の提出 407091 と同じ形。
// 基底を入れた順に並べておき、z = min(z, z ^ b) を順に当てる。b の最上位の bit が z に立っているときだけ z ^ b < z なので、
// その bit が消える。あとから入った元は、前の元の最上位の bit をどれも持たない (入れるときに消してある) ので、消した bit が
// また立つことはない。U と V は入れ替えても共通部分は同じなので、元の多いほうを先に基底にし、それが 30 本 (全空間) なら
// 共通部分はもう片方そのもの。元は (x << 32) | t の形で持つ (上 32 bit で消し、下 32 bit に V の元の和をためる)。
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 if(n < m) swap(u, v), swap(n, m);
 if(m == 0) return 0;
 if(n == 30) {
  for(int i= 0; i < m; ++i) w[i]= v[i];
  return m;
 }
 u64 basis[60];
 int r= 0, k= 0;
 for(int i= 0; i < n; ++i) {
  u64 z= u64(u[i]) << 32;
  for(int j= 0; j < r; ++j) z= min(z, z ^ basis[j]);
  basis[r++]= z;
 }
 for(int i= 0; i < m; ++i) {
  u64 z= (u64(v[i]) << 32) | v[i];
  for(int j= 0; j < r; ++j) z= min(z, z ^ basis[j]);
  if(z >> 32) basis[r++]= z;
  else w[k++]= u32(z);
 }
 return k;
}
