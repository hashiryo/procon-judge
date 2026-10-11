#pragma once
// 疎行列の行列式を Wiedemann の方法で求める。法は 998244353。
// 乱択の対角行列 D を右から掛けた B = A D は、A が正則ならほぼ確実に固有値がすべて異なり、最小多項式が特性多項式に一致する。
// 乱択の u、v で s_i = u^T B^i v (i < 2N) を作り、Berlekamp–Massey で最小の漸化式 s_n + C[1] s_{n-1} + ... + C[L] s_{n-L} = 0
// を求める。L = N なら det(B) = (-1)^N C[N]、L < N なら A はほぼ確実に正則でないので 0 を返す。det(A) = det(B) / det(D)。
// B w は、行を成分の数の多い順に並べ替え、「各行の j 番目の成分」を続けて並べた形 (jagged diagonal) で求める。j の段では成分を
// j 個より多く持つ行だけを順に回るので、内側のループに行の終わりの分岐がない。積は u64 にため、8 段ごとに 2^32 の位を畳む。
#include <chrono>
#include "../common.hpp"
#include "neo/algebra/ZMod.hpp"
inline u32 run(int N, int K, const vector<int>& a, const vector<int>& b, const vector<u32>& c) {
 using Z= ZMod<MOD>;
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
 auto rnd= [&]() { return u32(rng() % (MOD - 1) + 1); };
 constexpr u64 R= (u64(1) << 32) % MOD;
 auto fold= [](u64 x) { return (x >> 32) * R + (x & 0xffffffffu); };
 {  // 空の行か列があれば正則でない
  vector<char> hr(N), hc(N);
  for(int i= 0; i < K; ++i)
   if(c[i]) hr[a[i]]= hc[b[i]]= 1;
  for(int i= 0; i < N; ++i)
   if(!hr[i] || !hc[i]) return 0;
 }
 vector<u32> d(N);
 for(auto& x : d) x= rnd();
 // perm[q] は成分の数で q 番目に多い行。段 j には、成分を j 個より多く持つ行 q < cnt[j] の j 番目の成分が jst[j] から並ぶ。
 vector<int> len(N), perm(N), rank(N);
 for(int i= 0; i < K; ++i) ++len[a[i]];
 iota(perm.begin(), perm.end(), 0);
 stable_sort(perm.begin(), perm.end(), [&](int x, int y) { return len[x] > len[y]; });
 for(int q= 0; q < N; ++q) rank[perm[q]]= q;
 const int maxlen= N ? len[perm[0]] : 0;
 vector<int> cnt(maxlen), jst(maxlen + 1), col(K), fill_(N);
 for(int q= 0; q < N; ++q)
  for(int j= 0; j < len[perm[q]]; ++j) ++cnt[j];
 for(int j= 0; j < maxlen; ++j) jst[j + 1]= jst[j] + cnt[j];
 vector<u32> val(K);
 for(int i= 0; i < K; ++i) {
  const int q= rank[a[i]], e= jst[fill_[q]++] + q;
  col[e]= b[i], val[e]= (Z::raw(c[i]) * Z::raw(d[b[i]])).val();
 }
 vector<u64> acc(N);
 vector<u32> u(N), w(N), w2(N), s(2 * N);
 for(auto& x : u) x= rnd();
 for(auto& x : w) x= rnd();
 for(int i= 0; i < 2 * N; ++i) {
  u64 dot= 0;
  for(int r= 0; r < N; ++r) {
   dot+= u64(u[r]) * w[r];
   if((r & 7) == 7) dot= fold(dot);
  }
  s[i]= Z(dot).val();
  if(i + 1 == 2 * N) break;
  fill(acc.begin(), acc.end(), 0);
  for(int j= 0; j < maxlen; ++j) {
   const int cj= cnt[j], *cp= &col[jst[j]];
   const u32* vp= &val[jst[j]];
   for(int q= 0; q < cj; ++q) acc[q]+= u64(vp[q]) * w[cp[q]];
   if((j & 7) == 7)
    for(int q= 0; q < cj; ++q) acc[q]= fold(acc[q]);
  }
  for(int q= 0; q < N; ++q) w2[perm[q]]= Z(acc[q]).val();
  swap(w, w2);
 }
 // Berlekamp–Massey。C は今の漸化式 (次数 L)、B は直前に L が変わったときの漸化式、bb はそのときのずれ。
 vector<u32> C(N + 1), B(N + 1), T;
 C[0]= B[0]= 1;
 int L= 0, LB= 0, m= 1;
 Z bb= Z::raw(1);
 for(int n= 0; n < 2 * N; ++n) {
  u64 acc= 0;
  for(int i= 0; i <= L; ++i) {
   acc+= u64(C[i]) * s[n - i];
   if((i & 7) == 7) acc= fold(acc);
  }
  const Z dd= Z(acc);
  if(dd == Z()) {
   ++m;
   continue;
  }
  const Z coef= dd * bb.inv();
  const bool grow= 2 * L <= n;
  if(grow) T.assign(C.begin(), C.begin() + L + 1);
  for(int i= 0; i <= LB && i + m <= N; ++i) C[i + m]= (Z::raw(C[i + m]) - coef * Z::raw(B[i])).val();
  if(grow) {
   fill(B.begin(), B.begin() + LB + 1, 0u);
   copy(T.begin(), T.end(), B.begin());
   LB= L, L= n + 1 - L, bb= dd, m= 1;
  } else ++m;
 }
 if(L != N) return 0;
 Z dB= Z::raw(C[N]);
 if(N & 1) dB= -dB;
 Z dd= Z::raw(1);
 for(u32 x : d) dd*= Z::raw(x);
 return (dB * dd.inv()).val();
}
