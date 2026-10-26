#pragma once
// NeoLibrary の XorBasis と intersection (neo/linalg/XorBasis.hpp)。U と V の元を XorBasis に入れてから共通部分を求める。
// intersection は zassenhaus_min と同じ Zassenhaus の方法で、V の元を要の小さい順に流すので、出てくる元がそのまま基底の形になる。
#include "../common.hpp"
#include "neo/linalg/XorBasis.hpp"
inline int run(const u32* u, int n, const u32* v, int m, u32* w) {
 XorBasis<u32> a, b;
 for(int i= 0; i < n; ++i) a.insert(u[i]);
 for(int i= 0; i < m; ++i) b.insert(v[i]);
 const XorBasis<u32> c= intersection(a, b);
 for(int i= 0; i < c.size(); ++i) w[i]= c[i];
 return c.size();
}
