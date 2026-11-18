#pragma once
// 書き比べ: 速い核の手直しの試し (ranked_xor_v2.hpp) に、exp、log、合成、power projection の上の段を列ごとに済ませる形を足す。
// 変換の領域の各点の計算は位置ごとに閉じていて、区画をまたぐ段 (i - 4 > KC) では位置の塊 (h << KC) + j の更新が列 j の中で閉じる。
// 今の形は段ごとに大きい配列 (exp の G など) を全部読み書きするが、列 j の上の段を全部続けて作業の配列 (L2) の中で済ませれば、
// 大きい配列は、上の段の f̂_i の区画の変換 (書いて 1 回読む) と、最後の答えの区画の変換 (書いて 1 回読む) だけになる。
// f̂_i の区画の変換と下の段の結果は、答えの大きい配列の同じ列の空いた塊に置く (列 j の塊は列 j を済ませるときにだけ読み書きする)。
// 比べ終えたら、採るものを NeoLibrary に入れて消す。
#include "ranked_xor_v2.hpp"
namespace sps_xor2 {
// f̂_i (rank 0..i、roff = sb = 0) の列 j の上の段 (i - 4 - KC 段) の前向きと逆向きの変換。0 段なら塊を写すだけ (fwd_column と
// inv_column は 1 段以上を受ける)。
inline void col_fwd(u32* c, const u32* X, std::size_t j, int i, std::size_t bs, const MK& k) {
 if(i - 4 == KC) std::memcpy(c, X + j * bs, std::size_t(i + 1) * 32);
 else fwd_column(c, X, j, i, 0, 0, i + 1, bs, k);
}
inline void col_inv(u32* c, u32* X, std::size_t j, int i, std::size_t bs, const MK& k) {
 if(i - 4 == KC) std::memcpy(X + j * bs, c, std::size_t(i + 1) * 32);
 else inv_column(c, X, j, i, 0, 0, i + 1, bs, k);
}
// ---- exp (列ごと) ----
// G の列 j の塊 (h << KC) + j のうち、h = 2^H - 1 に下の段の G を、h ∈ [2^t - 1, 2^{t+1} - 1) に f̂_{KC+4+t} の区画の変換を置く
// (どちらも塊の幅は G と同じ)。列 j を済ませると、逆変換の上の段の結果を列 j の全部の塊に書く。
template <int N> void exp_col_n(const u32* b, u32* out, const MK& k0) {
 constexpr int NS= N, BS= NS * 8, NB= N - 4;
 if constexpr(NB <= KC) {
  exp_n<N>(b, out, k0);
 } else {
  const MK k= k0;
  constexpr int H= NB - KC;
  constexpr std::size_t CB= std::size_t(1) << KC;
  u32 *G= huge_alloc((std::size_t(1) << NB) * BS * 4), *bs= huge_alloc(std::size_t(4) << N);
  scale_r(b, bs, std::size_t(1) << N, k);
  u32* Glo= G + ((std::size_t(1) << H) - 1) * CB * BS;
  auto Fi= [&](int i) { return G + ((std::size_t(1) << (i - 4 - KC)) - 1) * CB * BS; };
  {
   alignas(32) u32 h[16], bb[16];
   for(int s= 0; s < 16; ++s) bb[s]= b[s] % k.P;
   naive_exp(4, bb, h, k.P);
   init_block(Glo, h, 0, 1, NS, k);
  }
  // 下の段 (区画の中で閉じる)
  u32* Fs= small_alloc((CB >> 1) * BS * 4);
  for(int i= 4; i < KC + 4; ++i) {
   const std::size_t cnt= std::size_t(1) << (i - 4);
   fwd_chunks(Fs, bs + (std::size_t(1) << i), i, 0, 0, i + 1, BS, k);
   for(std::size_t q= 0; q < cnt; ++q) exp_step(Glo + q * BS, Glo + (q + cnt) * BS, Fs + q * BS, i, k);
  }
  std::free(Fs);
  // 上の段の f̂_i の区画の変換
  for(int i= KC + 4; i < N; ++i) fwd_chunks(Fi(i), bs + (std::size_t(1) << i), i, 0, 0, i + 1, BS, k);
  // 列ごとに上の段を全部済ませ、逆変換の上の段を G に書く
  u32 *cG= small_alloc((std::size_t(1) << H) * BS * 4), *cF= small_alloc((std::size_t(1) << (H - 1)) * BS * 4);
  for(std::size_t j= 0; j < CB; ++j) {
   std::memcpy(cG, Glo + j * BS, std::size_t(BS) * 4);
   for(int i= KC + 4; i < N; ++i) {
    const std::size_t hs= std::size_t(1) << (i - 4 - KC);
    col_fwd(cF, Fi(i), j, i, BS, k);
    for(std::size_t h= 0; h < hs; ++h) exp_step(cG + h * BS, cG + (h + hs) * BS, cF + h * BS, i, k);
   }
   inv_column(cG, G, j, N, 0, 1, NS, BS, k);
  }
  const V Kc= cst(u32(u64(k.inv2pow(N - 1)) * k.R1 % k.P));
  inv_chunks(G, N, 0, 1, NS, BS, k, [&](std::size_t q, const V* v) {
   for(int t= 0; t < 2; ++t) _mm256_storeu_si256((V*)(out + 16 * q + 8 * t), norm1(mont(v[t], Kc, k), k));
  });
  out[0]= 1;
  std::free(G), std::free(bs), std::free(cG), std::free(cF);
 }
}
// ---- log (列ごと) ----
// 大きい配列 Fb の列 j の塊のうち、h = 2^H - 1 に下の段の B を、h ∈ [2^t - 1, 2^{t+1} - 1) に段 KC+4+t の f̂ の区画の変換を置く。
// 列 j では、B の列を作業の配列で伸ばしながら上の段の商を求め、商の列を逆変換の上の段まで済ませて Fb の同じ場所に戻す。
// 列を全部済ませてから、上の段ごとに区画の逆変換をして答えを取り出す。
template <int N> void log_col_n(const u32* h, u32* out, const MK& k0) {
 constexpr int NSB= N - 1, BSB= NSB * 8, NSF= N, BSF= NSF * 8, NB= N - 4;
 if constexpr(NB <= KC) {
  log_n<N>(h, out, k0);
 } else {
  const MK k= k0;
  constexpr int H= NB - KC;
  constexpr std::size_t CB= std::size_t(1) << KC;
  u32 *Fb= huge_alloc((std::size_t(1) << NB) * BSF * 4), *hs= huge_alloc(std::size_t(4) << N);
  scale_r(h, hs, std::size_t(1) << N, k);
  {
   u32 hh[16];
   for(int s= 0; s < 16; ++s) hh[s]= h[s] % k.P;
   naive_log(4, hh, out, k.P);
  }
  u32* Blo= Fb + ((std::size_t(1) << H) - 1) * CB * BSF;
  auto Fi= [&](int i) { return Fb + ((std::size_t(1) << (i - 4 - KC)) - 1) * CB * BSF; };
  init_block(Blo, hs, 0, 1, NSB, k);
  // 下の段 (区画の中で閉じる)。B の塊の幅は BSF
  u32* Fs= small_alloc((CB >> 1) * BSF * 4);
  for(int i= 4; i < KC + 4; ++i) {
   const int ns= i + 1;
   const std::size_t cnt= std::size_t(1) << (i - 4);
   fwd_chunks(Fs, hs + (std::size_t(1) << i), i, 0, 0, ns, BSF, k);
   for(std::size_t q= 0; q < cnt; ++q) div_step(Fs + q * BSF, Blo + q * BSF, Blo + (q + cnt) * BSF, i, true, k);
   const V Kc= cst(k.inv2pow(i - 1));
   u32* o= out + (std::size_t(1) << i);
   inv_chunks(Fs, i, 0, 0, ns, BSF, k, [&](std::size_t q, const V* v) {
    for(int t= 0; t < 2; ++t) _mm256_storeu_si256((V*)(o + 16 * q + 8 * t), norm1(mont(v[t], Kc, k), k));
   });
  }
  std::free(Fs);
  for(int i= KC + 4; i < N; ++i) fwd_chunks(Fi(i), hs + (std::size_t(1) << i), i, 0, 0, i + 1, BSF, k);
  u32 *cB= small_alloc((std::size_t(1) << (H - 1)) * BSB * 4), *cF= small_alloc((std::size_t(1) << (H - 1)) * BSF * 4);
  for(std::size_t j= 0; j < CB; ++j) {
   std::memcpy(cB, Blo + j * BSF, std::size_t(BSB) * 4);
   for(int i= KC + 4; i < N; ++i) {
    const std::size_t hc= std::size_t(1) << (i - 4 - KC);
    const bool upd= i + 1 < N;
    col_fwd(cF, Fi(i), j, i, BSF, k);
    for(std::size_t t= 0; t < hc; ++t) div_step(cF + t * BSF, cB + t * BSB, cB + (t + hc) * BSB, i, upd, k);
    col_inv(cF, Fi(i), j, i, BSF, k);
   }
  }
  for(int i= KC + 4; i < N; ++i) {
   const V Kc= cst(k.inv2pow(i - 1));
   u32* o= out + (std::size_t(1) << i);
   inv_chunks(Fi(i), i, 0, 0, i + 1, BSF, k, [&](std::size_t q, const V* v) {
    for(int t= 0; t < 2; ++t) _mm256_storeu_si256((V*)(o + 16 * q + 8 * t), norm1(mont(v[t], Kc, k), k));
   });
  }
  std::free(Fb), std::free(hs), std::free(cB), std::free(cF);
 }
}
// ---- 合成 (列ごと) ----
// G_0 の大きい配列の列 j の塊のうち、h = 2^H - 1 に下の段の G_0 を、h ∈ [2^t - 1, 2^{t+1} - 1) に ĝ_{KC+4+t} の区画の変換を置く。
// G_j (j >= 1) の下の段は別の配列 (塊 2^KC 個まで)。列 j では G_0..G_H の列を作業の配列で伸ばして上の段を済ませる。
template <int N> void egf_comp_col_n(const u32* g, const u32* e, u32* out, const MK& k0) {
 constexpr int NB= N - 4;
 if constexpr(NB <= KC) {
  egf_comp_n<N>(g, e, out, k0);
 } else {
  const MK k= k0;
  constexpr int H= NB - KC, NS= N + 1;
  constexpr std::size_t CB= std::size_t(1) << KC, BS= NS * 8;
  u32* gs= huge_alloc(std::size_t(4) << N);
  scale_r(g, gs, std::size_t(1) << N, k);
  u32* G0= huge_alloc((std::size_t(1) << NB) * BS * 4);
  u32* G[N - 3];
  std::size_t bsj[N - 3];
  for(int j= 0; j <= N - 4; ++j) {
   bsj[j]= std::size_t(N - j + 1) * 8;
   G[j]= j == 0 ? G0 + ((std::size_t(1) << H) - 1) * CB * BS : huge_alloc(std::min(std::size_t(1) << (N - j - 4), CB) * bsj[j] * 4);
  }
  auto Fi= [&](int i) { return G0 + ((std::size_t(1) << (i - 4 - KC)) - 1) * CB * BS; };
  {
   std::vector<std::vector<u32>> h(N + 1, std::vector<u32>(16, 0));
   for(int j= 0; j <= N; ++j) h[j][0]= e[j];
   for(int i= 0; i < 4; ++i)
    for(int j= 0; j + i < N; ++j)
     for(int s= 0; s < (1 << i); ++s) {
      u64 t= 0;
      for(int a= s;; a= (a - 1) & s) {
       t= (t + u64(h[j + 1][a]) * (g[(1 << i) | (s ^ a)] % k.P)) % k.P;
       if(!a) break;
      }
      h[j][(1 << i) | s]= u32(t);
     }
   for(int j= 0; j <= N - 4; ++j) init_block(G[j], h[j].data(), 0, 0, 5, k);
  }
  constexpr std::size_t BSF= N * 8;
  u32* Fs= small_alloc((CB >> 1) * BSF * 4);
  u32 *gp[N - 3], *gh[N - 3];
  for(int i= 4; i < KC + 4; ++i) {
   const int J= N - i;
   const std::size_t cnt= std::size_t(1) << (i - 4);
   fwd_chunks(Fs, gs + (std::size_t(1) << i), i, 0, 0, i + 1, BSF, k);
   for(std::size_t q= 0; q < cnt; ++q) {
    for(int j= 0; j <= J; ++j) gp[j]= G[j] + q * bsj[j];
    for(int j= 0; j < J; ++j) gh[j]= G[j] + (q + cnt) * bsj[j];
    comp_step(gp, gh, Fs + q * BSF, i, J, k);
   }
  }
  std::free(Fs);
  for(int i= KC + 4; i < N; ++i) fwd_chunks(Fi(i), gs + (std::size_t(1) << i), i, 0, 0, i + 1, BS, k);
  // 列の作業の配列。G_j の列は塊 2^{H-j} 個
  u32* cG[H + 1];
  for(int j= 0; j <= H; ++j) cG[j]= small_alloc((std::size_t(1) << (H - j)) * bsj[j] * 4);
  u32* cF= small_alloc((std::size_t(1) << (H - 1)) * BS * 4);
  for(std::size_t c= 0; c < CB; ++c) {
   for(int j= 0; j <= H; ++j) std::memcpy(cG[j], G[j] + c * bsj[j], bsj[j] * 4);
   for(int i= KC + 4; i < N; ++i) {
    const int J= N - i;
    const std::size_t hc= std::size_t(1) << (i - 4 - KC);
    col_fwd(cF, Fi(i), c, i, BS, k);
    for(std::size_t t= 0; t < hc; ++t) {
     for(int j= 0; j <= J; ++j) gp[j]= cG[j] + t * bsj[j];
     for(int j= 0; j < J; ++j) gh[j]= cG[j] + (t + hc) * bsj[j];
     comp_step(gp, gh, cF + t * BS, i, J, k);
    }
   }
   inv_column(cG[0], G0, c, N, 0, 0, NS, BS, k);
  }
  const V Kc= cst(u32(u64(k.inv2pow(N - 1)) * k.R1 % k.P));
  inv_chunks(G0, N, 0, 0, NS, BS, k, [&](std::size_t q, const V* v) {
   for(int t= 0; t < 2; ++t) _mm256_storeu_si256((V*)(out + 16 * q + 8 * t), norm1(mont(v[t], Kc, k), k));
  });
  out[0]= e[0];
  for(int j= 1; j <= N - 4; ++j) std::free(G[j]);
  for(int j= 0; j <= H; ++j) std::free(cG[j]);
  std::free(G0), std::free(cF), std::free(gs);
 }
}
// ---- 合成の転置 (power projection、列ごと) ----
// w の区画の変換 W と、上の段の ĝ_i の区画の変換 (別の大きい配列 Fb の行 [2^t - 1, 2^{t+1} - 1)) を先に作る。列 j では W の列を
// 前向きの上の段まで済ませて D_0 の列にし、上の段 (i = N-1..KC+4) を作業の配列で済ませて、残った D_0..D_H の塊 1 つずつを下の段の
// 配列の塊 j に書く。下の段は今の形で行う。
template <int N> std::vector<u32> egf_proj_col_n(const u32* g, const u32* w, const MK& k0) {
 constexpr int NB= N - 4, NS= N + 1;
 if constexpr(NB <= KC) {
  return egf_proj_n<N>(g, w, k0);
 } else {
  const MK k= k0;
  constexpr int H= NB - KC;
  constexpr std::size_t CB= std::size_t(1) << KC, BS= NS * 8, BSF= N * 8;
  u32* gs= huge_alloc(std::size_t(4) << N);
  scale_r(g, gs, std::size_t(1) << N, k);
  u32 *W= huge_alloc((std::size_t(1) << NB) * BS * 4), *Fb= huge_alloc(((std::size_t(1) << H) - 1) * CB * BSF * 4);
  auto Fi= [&](int i) { return Fb + ((std::size_t(1) << (i - 4 - KC)) - 1) * CB * BSF; };
  u32* D[N - 3];
  std::size_t bsj[N - 3];
  for(int j= 0; j <= N - 4; ++j) bsj[j]= std::size_t(N - j + 1) * 8, D[j]= huge_alloc(std::min(std::size_t(1) << (N - j - 4), CB) * bsj[j] * 4);
  fwd_chunks(W, w, N, 0, 0, NS, BS, k);
  for(int i= KC + 4; i < N; ++i) fwd_chunks(Fi(i), gs + (std::size_t(1) << i), i, 0, 0, i + 1, BSF, k);
  u32* cD[H + 1];
  for(int j= 0; j <= H; ++j) cD[j]= small_alloc((std::size_t(1) << (H - j)) * bsj[j] * 4);
  u32* cF= small_alloc((std::size_t(1) << (H - 1)) * BSF * 4);
  u32 *zl[N - 3], *zh[N - 3], *yy[N - 3];
  for(std::size_t c= 0; c < CB; ++c) {
   fwd_column(cD[0], W, c, N, 0, 0, NS, BS, k);
   for(int i= N - 1; i >= KC + 4; --i) {
    const int J= N - i - 1;
    const std::size_t hc= std::size_t(1) << (i - 4 - KC);
    col_fwd(cF, Fi(i), c, i, BSF, k);
    for(std::size_t t= 0; t < hc; ++t) {
     for(int j= 0; j <= J; ++j) zl[j]= cD[j] + t * bsj[j], zh[j]= cD[j] + (t + hc) * bsj[j];
     for(int j= 0; j <= J + 1; ++j) yy[j]= cD[j] + t * bsj[j];
     proj_step(zl, zh, yy, cF + t * BSF, i, J + 1, k);
    }
   }
   for(int j= 0; j <= H; ++j) std::memcpy(D[j] + c * bsj[j], cD[j], bsj[j] * 4);
  }
  u32* Fs= small_alloc((CB >> 1) * BSF * 4);
  for(int i= KC + 3; i >= 4; --i) {
   const int J= N - i - 1;
   const std::size_t cnt= std::size_t(1) << (i - 4);
   fwd_chunks(Fs, gs + (std::size_t(1) << i), i, 0, 0, i + 1, BSF, k);
   for(std::size_t q= 0; q < cnt; ++q) {
    for(int j= 0; j <= J; ++j) zl[j]= D[j] + q * bsj[j], zh[j]= D[j] + (q + cnt) * bsj[j];
    for(int j= 0; j <= J + 1; ++j) yy[j]= D[j] + q * bsj[j];
    proj_step(zl, zh, yy, Fs + q * BSF, i, J + 1, k);
   }
  }
  // 段 4 の双対 (各 j に塊 1 つ、rank 0..4) を 4 bit の素朴な形に戻し、g^m / m! と内積を取る
  std::vector<std::vector<u32>> cc= egf_powers(4, g, k.P);
  std::vector<u32> vk(N + 1, 0);
  for(int j= 0; j <= N - 4; ++j) {
   alignas(32) u32 v[5][8], gt[16];
   for(int r= 0; r <= 4; ++r) st(v[r], norm1(lane_wht(ld(D[j] + 8 * r), k), k));
   for(int s= 0; s < 16; ++s) gt[s]= v[__builtin_popcount(s)][s >> 1];
   for(int m= 0; m <= 4 && j + m <= N; ++m) {
    u64 t= 0;
    for(int s= 0; s < 16; ++s) t= (t + u64(gt[s]) * cc[m][s]) % k.P;
    vk[j + m]= u32((vk[j + m] + t) % k.P);
   }
  }
  const u64 sc= k.inv2pow(N - 1);
  for(auto& x: vk) x= u32(x * sc % k.P);
  for(int j= 0; j <= N - 4; ++j) std::free(D[j]);
  for(int j= 0; j <= H; ++j) std::free(cD[j]);
  std::free(W), std::free(Fb), std::free(Fs), std::free(cF), std::free(gs);
  return vk;
 }
}
}  // namespace sps_xor2
