#pragma once
// v46 (対称性なしの BSGS を constexpr で焼き、solve から詰め直しを抜いた版) の前半、
// x^(2^32-1) の作り方を割り算経由に替えた版。
//
// 641 と 6700417 を取り出すには x^(2^32-1) が要る。v46 までは Itoh-Tsujii と同じ形で
// x^3 → x^15 → x^255 → x^(2^16-1) と 4 回掛けてから 2^16+1 乗していたが、
// x^(2^32-1) = frob32(x) / x なので、逆元さえあれば掛け算 2 回で済む。逆元は
// gf2-64-div の subfield_split_7 と同じやり方で、N = x^(2^32+1) から x^M ∈ F_2^16 に
// 落として 16 bit の逆元表を 1 回引く形にした。x^M は F_2^16 側の log でも使うので、
// もともと計算しているものをそのまま使い回せる。
//
// 65537 側は比率方式にした。v46 の y = s + s^-1 方式は x^(2^32-1) とは別に
// x_65537 = s^(2^32+1) も要るが、比率方式なら N と frob16(N) だけで引けるので、
// 消した鎖を復活させずに済む。
//
// 掛け算は 7 本から 5 本、frob は 7 本から 2 本に減る代わりに、128 KB の逆元表の引きが
// 1 回増える。
//
// 必要な拡張: VPCLMULQDQ + AVX2 (Intel Ice Lake / AMD Zen3 以降).
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_log_pohlig_v47 {
using gf2_64_pclmul::frob10;
using gf2_64_pclmul::frob16;
using gf2_64_pclmul::frob2;
using gf2_64_pclmul::frob3;
using gf2_64_pclmul::frob32;
using gf2_64_pclmul::frob4;
using gf2_64_pclmul::frob6;
using gf2_64_pclmul::frob7;
using gf2_64_pclmul::frob8;
using gf2_64_pclmul::mul;
using gf2_64_pclmul::mul2;
using gf2_64_pclmul::sq;
using gf2_64_pclmul::unpack;
// =============================================================================
// constexpr GF(2^64) 乗算 (PerfectHash641 の build 用).
// GNU_TARGET("pclmul") intrinsic は constexpr 化できないため、4-bit windowed CLMUL で実装。
// reduction polynomial は runtime 版と同一: x^64 + x^4 + x^3 + x + 1 (R = 0x1B).
// =============================================================================
constexpr void clmul128_ce(u64 a, u64 b, u64& lo_out, u64& hi_out) {
 u64 Tlo[16]= {0}, Thi[16]= {0};
 Tlo[1]= a;
 for(int v= 2; v < 16; ++v) {
  u64 plo= Tlo[v >> 1], phi= Thi[v >> 1];
  u64 nlo= plo << 1;
  u64 nhi= (phi << 1) | (plo >> 63);
  if(v & 1) nlo^= a;
  Tlo[v]= nlo;
  Thi[v]= nhi;
 }
 u64 lo= 0, hi= 0;
 for(int s= 60; s >= 0; s-= 4) {
  u64 nhi= (hi << 4) | (lo >> 60);
  u64 nlo= lo << 4;
  u32 nib= u32((b >> s) & 0xF);
  lo= nlo ^ Tlo[nib];
  hi= nhi ^ Thi[nib];
 }
 lo_out= lo;
 hi_out= hi;
}
constexpr u64 mul_ce(u64 a, u64 b) {
 u64 lo, hi;
 clmul128_ce(a, b, lo, hi);
 constexpr u64 R= 0x1B;
 u64 fold1_lo, fold1_hi;
 clmul128_ce(hi, R, fold1_lo, fold1_hi);
 u64 fold2_lo, fold2_hi;
 clmul128_ce(fold1_hi, R, fold2_lo, fold2_hi);
 return lo ^ fold1_lo ^ fold2_lo;
}
// =============================================================================
// F_{2^16}^* log table (compile-time 構築) - 元の v11 から変更なし
// =============================================================================
struct Ln16Table {
 u16 t[65536];
};
constexpr Ln16Table LN16= []() {
 u16 col[]= {1U, 11778U, 7028U, 51115U, 48663U, 26081U, 17458U, 40223U, 30334U, 42368U, 14380U, 2223U, 49688U, 11217U, 44239U, 63445U};
 u16 T_lo[256]= {}, T_hi[256]= {};
 for(int v= 0; v < 256; ++v) {
  u16 lo= 0, hi= 0;
  for(int j= 0; j < 8; ++j)
   if((v >> j) & 1) {
    lo^= col[j];
    hi^= col[j + 8];
   }
  T_lo[v]= lo;
  T_hi[v]= hi;
 }
 Ln16Table ln{};
 u16 cur= 1;
 for(u32 k= 0; k < 65535; ++k) {
  u16 lo= T_lo[u8(cur)] ^ T_hi[cur >> 8];
  ln.t[lo]= u32(u16(k)) * 2699 % 65535;
  cur= u16(cur << 1) ^ (0x002DU & -u16(cur >> 15));
 }
 ln.t[0]= 0;
 return ln;
}();
// =============================================================================
// 定数群
// =============================================================================
constexpr u64 P_F16= 65535;
constexpr u64 P_641= 641;
constexpr u64 P_F17= 65537;
constexpr u64 P_BIG= 6700417;
constexpr u64 INV_65535_641= 243ULL;
constexpr u64 MOD2= (P_F16 * P_641) % P_F17;
constexpr u64 INV_MOD2_F17= 45242ULL;
constexpr u64 MOD3= (P_F16 * P_641 * P_F17) % P_BIG;
constexpr u64 INV_MOD3_BIG= 3883315ULL;
constexpr u64 MOD_F16= P_F16;
constexpr u64 MOD_F16_641= P_F16 * P_641;
constexpr u64 MOD_F16_641_F17= P_F16 * P_641 * P_F17;
constexpr u64 EXP_F16= 0x0001000100010001ull;
constexpr u64 EXP_641= 0x00663d80ff99c27full;
constexpr u64 EXP_F17= 0x0000ffff0000ffffull;
constexpr u64 EXP_BIG= 0x00000280fffffd7full;
constexpr u64 G_641= 0x6bf808f7824282a2ull;
constexpr u64 G_65537= 0x1c1e79669b95a7ceull;
constexpr u64 G_6700417= 0x00f542601703f991ull;
// =============================================================================
// PerfectHash641: 641 元 subgroup 用 完全ハッシュテーブル (constexpr).
//
// ハッシュ関数: h(x) = (x * C) >> (64 - BITS), Fibonacci-style.
// C = 0xffef5fb99f1bf6e7 は ~50 万ランダム試行で衝突ゼロを発見した定数。
// cap = 2^14 = 16384, u16 entries -> table size = 32 KB.
//
// lookup target は必ず order-641 subgroup の元なので、未登録 key を引くことはない。
// したがって empty marker / fp check は不要、tab[hash(t)] を直接返すだけ。
// =============================================================================
struct PerfectHash641 {
 static constexpr u64 C= 0xffef5fb99f1bf6e7ull;
 static constexpr u32 BITS= 14;
 static constexpr u32 CAP= 1u << BITS;
 static constexpr u32 SHIFT= 64 - BITS;
 static constexpr array<u16, CAP> tab= []() {
  array<u16, CAP> t{};
  u64 cur= 1;
  for(u32 k= 0; k < P_641; ++k) {
   t[u32((cur * C) >> SHIFT)]= u16(k);
   cur= mul_ce(cur, G_641);
  }
  return t;
 }();
 static u32 lookup(u64 t) { return tab[u32((t * C) >> SHIFT)]; }
};
// =============================================================================
// ClassTable65537: 65537 元 subgroup 用 直接引き表 (constexpr, 256 KB).
//
// N = x^(2^32+1) (F_2^32 の元) の F_2^16 座標の比を添字にする。φ を φ + frob16(φ) = 1 に
// 取ると N = b_0 + φ b_1 と分かれ、b_1 = N + frob16(N)、b_0 = N + φ b_1 で座標が出る。
// N に F_2^16 の元を掛けても μ_P 成分 u は変わらないので、u は比 b_0 : b_1 で決まる。
// 比の log (F_2^16 の log 表の差) をそのまま添字にする。欲しいのは
// x_65537 = N^65535 = u^-2 の log なので、表には (-2 log u) mod 65537 が入っている。
//
// 表は生配列で持つこと。65537 回の walk を std::array の operator[] で回すと clang の
// constexpr step 上限に当たる。
// =============================================================================
struct RawByteTable {
 u64 t[8][256];
};
constexpr u64 apply_raw(const RawByteTable& T, u64 a) { return T.t[0][u8(a)] ^ T.t[1][u8(a >> 8)] ^ T.t[2][u8(a >> 16)] ^ T.t[3][u8(a >> 24)] ^ T.t[4][u8(a >> 32)] ^ T.t[5][u8(a >> 40)] ^ T.t[6][u8(a >> 48)] ^ T.t[7][u8(a >> 56)]; }
// G_65537 倍。基底の像は x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
// c 倍の byte table。基底の像は x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
constexpr RawByteTable make_mul_table(u64 c) {
 u64 basis[64]{};
 basis[0]= c;
 for(int i= 1; i < 64; ++i) basis[i]= (basis[i - 1] << 1) ^ (IRRED_LOW & -(basis[i - 1] >> 63));
 RawByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int j= 0; j < 8; ++j) {
   const u64 v= basis[8 * p + j];
   for(int b= 0; b < (1 << j); ++b) r.t[p][(1 << j) | b]= r.t[p][b] ^ v;
  }
 return r;
}
constexpr RawByteTable MUL_G17= make_mul_table(G_65537);
// frob.hpp の表は std::array なので、walk で引く用に生配列へ写しておく。
constexpr RawByteTable FROB16_RAW= []() {
 RawByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int b= 0; b < 256; ++b) r.t[p][b]= gf2_64_pclmul::FROB16_BYTE[p][b];
 return r;
}();
// φ: Tr_{F_2^32/F_2^16}(φ) = 1 を満たす元 (N = b_0 + φ b_1 の φ)
constexpr u64 PHI= 0x025bb4340671c0c5ull;
static_assert((PHI ^ apply_raw(FROB16_RAW, PHI)) == 1, "PHI の相対トレースが 1 でない");
// LC.t[h][i]: φ 倍の下位 16 bit (入力は F_2^16 の識別子)
struct HalfTable {
 u16 t[2][256];
};
constexpr HalfTable LC= []() {
 // SUBFIELD_BASIS[i] は下位 16 bit がちょうど 1 << i の F_2^16 の元 (識別子 1 << i)
 u64 SUBFIELD_BASIS[]= {0x0000000000000001ull, 0x5fbfaec6aeac0002ull, 0xb06c601895640004ull, 0xb013b5277b7c0008ull, 0xb5ebb915248a0010ull, 0x109bb25b2c600020ull, 0xbf3bd95bd4190040ull, 0x0fc66342279b0080ull, 0xb6418f5e57c50100ull, 0xaa194bd4b83f0200ull, 0x1b5217b4dcc70400ull, 0xbb06fa73867a0800ull, 0x006fd55b23331000ull, 0x4ae8fb39198c2000ull, 0xfbd141b29b4f4000ull, 0x1d9ce1776be78000ull};
 const RawByteTable m= make_mul_table(PHI);
 u16 basis[16]{};
 for(int i= 0; i < 16; ++i) basis[i]= u16(apply_raw(m, SUBFIELD_BASIS[i]));
 HalfTable t{};
 for(int half= 0; half < 2; ++half)
  for(int j= 0; j < 8; ++j)
   for(int b= 0; b < (1 << j); ++b) t.t[half][(1 << j) | b]= t.t[half][b] ^ basis[j + half * 8];
 return t;
}();
struct ClassTable65537 {
 u32 t[65535];
 u32 K0;
};
constexpr ClassTable65537 CLS65537= []() {
 ClassTable65537 r{};
 u64 cur= 1;
 u32 v= 0;  // (-2 k) mod 65537
 for(u32 k= 0; k < P_F17; ++k) {
  const u64 fr= apply_raw(FROB16_RAW, cur);
  const u16 b1= u16(cur ^ fr), b0= u16(u16(cur) ^ LC.t[0][u8(b1)] ^ LC.t[1][b1 >> 8]);
  if(b1 == 0) {         // 類 ∞ = F_2^16 自身。u = 1 なので log は 0
  } else if(b0 == 0) {  // もう 1 つの特別な類
   r.K0= v;
  } else {
   u32 idx= LN16.t[b0] + u32(P_F16) - LN16.t[b1];
   if(idx >= u32(P_F16)) idx-= u32(P_F16);
   r.t[idx]= v;
  }
  cur= apply_raw(MUL_G17, cur);
  v= v >= 2 ? v - 2 : v + u32(P_F17) - 2;
 }
 return r;
}();
// n = x^(2^32+1), fn = frob16(n) から log_{G_65537}(x^(2^32-1)) を返す
inline u32 log_65537(u64 n, u64 fn) {
 const u16 b1= u16(n ^ fn), b0= u16(u16(n) ^ LC.t[0][u8(b1)] ^ LC.t[1][b1 >> 8]);
 u32 idx= LN16.t[b0] + u32(P_F16) - LN16.t[b1];
 if(idx >= u32(P_F16)) idx-= u32(P_F16);
 u32 r= CLS65537.t[idx];
 if(!b0) r= CLS65537.K0;
 if(!b1) r= 0;
 return r;
}
// =============================================================================
// BSGSTable6700417: 6700417 元 subgroup 用 BSGS (packed layout).
//
// レイアウト: u64 entry = (key & HI_MASK) | value
//   - 下位 17 bit: value (j ∈ [0, m))
//   - bits 17..18: gap (常に 0)
//   - 上位 45 bit: key の fingerprint (= key & ~MASK)
//
// 比較は ((e ^ t) & HI_MASK) == 0 を、コンパイラは自動的に (e ^ t) <= MASK
// に書き換える (5 命令のホットループになる)。
//
// inv_base_m = G_6700417^(P_BIG - m) は事前計算した constexpr 定数を使用。
// =============================================================================
struct BSGSTable6700417 {
 static constexpr u32 mask= 524287;  // 19-bit, cap = 524288
 static constexpr u64 q= P_BIG;
 static constexpr u32 m= 131072;
 static constexpr u64 max_i= 52;
 static constexpr u64 HI_MASK= ~u64(mask);  // = 0xFFFFFFFFFFF80000
 static constexpr u64 inv_base_m= 0x1489880b9cf723deull;
 static constexpr u64 inv2_base_m= 0x5be693c8c2c557e3ull;            // = inv_base_m^2
 static constexpr u64 inv3_base_m= mul_ce(inv_base_m, inv2_base_m);  // = inv_base_m^3
 static constexpr u64 inv4_base_m= 0xfdb44dcbca6522deull;            // = inv_base_m^4  (4-streams 用)
 // 空きは 0。値の側を j + 1 にして実在の entry が 0 にならないようにしてある
 // (こうすると 52 万要素の初期化ループが要らず、constexpr の step も浮く)。
 struct Tab {
  u64 t[mask + 1];
 };
 static constexpr Tab TAB= []() {
  const RawByteTable mg= make_mul_table(G_6700417);
  Tab r{};
  u64 cur= 1;
  for(u32 j= 0; j < m; ++j) {
   u32 h= u32(cur) & mask;
   while(r.t[h]) h= (h + 1) & mask;
   r.t[h]= (cur & HI_MASK) | (u64(j) + 1);
   cur= apply_raw(mg, cur);
  }
  return r;
 }();
 // 定数のベクタは 1 度だけ作る (関数の中で組むと毎回即値の組み立てが走る)
 static inline const __m256i V_S01= _mm256_set_epi64x(0, inv_base_m, 0, 1);
 static inline const __m256i V_S23= _mm256_set_epi64x(0, inv3_base_m, 0, inv2_base_m);
 static inline const __m256i V_S4= _mm256_set1_epi64x(inv4_base_m);
 GNU_TARGET("pclmul,vpclmulqdq") static inline u32 solve(u64 target) {
  // stream は (0,1) と (2,3) の 2 本のベクタで持つ。mul2 の出力 (q0, q2) が次の mul2 の
  // operand の置き場所そのものなので、段を進めるのに詰め直しが要らない。
  const __m256i tv= _mm256_set1_epi64x(target);
  __m256i A= mul2(tv, V_S01), B= mul2(tv, V_S23);  // (t0, t1), (t2, t3)
  __m256i An= mul2(A, V_S4), Bn= mul2(B, V_S4);
  __m256i An2= mul2(An, V_S4), Bn2= mul2(Bn, V_S4);
  u64 t[4], t_n[4], t_n2[4];
  tie(t[0], t[1])= unpack(A), tie(t[2], t[3])= unpack(B);
  tie(t_n[0], t_n[1])= unpack(An), tie(t_n[2], t_n[3])= unpack(Bn);
  tie(t_n2[0], t_n2[1])= unpack(An2), tie(t_n2[2], t_n2[3])= unpack(Bn2);
  for(int j= 0; j < 4; ++j) {
   _mm_prefetch((const char*)&TAB.t[u32(t[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&TAB.t[u32(t_n[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&TAB.t[u32(t_n2[j]) & mask], _MM_HINT_T0);
  }
  for(u8 i= 0; i <= max_i; i+= 4) {
   for(int j= 0; j < 4; ++j) {
    if(i + j > max_i) break;
    u64 tt= t[j];
    u32 h= u32(tt) & mask;
    while(TAB.t[h]) {
     u64 e= TAB.t[h];
     if(((e ^ tt) & HI_MASK) == 0) return u32((i + j) * m + u32(e & mask) - 1);
     h= (h + 1) & mask;
    }
   }
   A= An, B= Bn, An= An2, Bn= Bn2;
   for(int j= 0; j < 4; ++j) t[j]= t_n[j], t_n[j]= t_n2[j];
   if(i + 12 <= max_i) {
    An2= mul2(An, V_S4), Bn2= mul2(Bn, V_S4);
    tie(t_n2[0], t_n2[1])= unpack(An2), tie(t_n2[2], t_n2[3])= unpack(Bn2);
    for(int j= 0; j < 4; ++j) _mm_prefetch((const char*)&TAB.t[u32(t_n2[j]) & mask], _MM_HINT_T0);
   }
  }
  return q;
 }
};
// 641 も 65537 も 6700417 も表は全部 constexpr なので、実行時の構築は無い。
// =============================================================================
// INV_LOW: F_2^16 の逆元表 (識別子 → 逆元の識別子)。gf2-64-div の subfield_split_7 と同じ。
// BETA の冪を歩いて k と 65535-k を対にするだけなので constexpr で焼ける。
// embed_idx は識別子から 64 bit の元へ戻す写像 (部分体の基底の XOR)。
// =============================================================================
constexpr auto INV_LOW= []() {
 u16 col[]= {1U, 11778U, 7028U, 51115U, 48663U, 26081U, 17458U, 40223U, 30334U, 42368U, 14380U, 2223U, 49688U, 11217U, 44239U, 63445U};
 u16 T_lo[256]= {}, T_hi[256]= {};
 for(int v= 0; v < 256; ++v) {
  u16 lo= 0, hi= 0;
  for(int j= 0; j < 8; ++j)
   if((v >> j) & 1) {
    lo^= col[j];
    hi^= col[j + 8];
   }
  T_lo[v]= lo;
  T_hi[v]= hi;
 }
 u16 nat[65535];
 for(u16 i= 0, cur= 1; i < 65535; ++i) nat[i]= cur, cur= u16(cur << 1) ^ (0x002DU & -u16(cur >> 15));
 array<u16, 65536> t{};
 const u16 lo1= T_lo[1] ^ T_hi[0];
 t[lo1]= lo1;
 for(u32 k= 1; k <= 32767; ++k) {
  const u16 nk= nat[k], nik= nat[65535 - k];
  const u16 lo_k= T_lo[u8(nk)] ^ T_hi[nk >> 8], lo_ik= T_lo[u8(nik)] ^ T_hi[nik >> 8];
  t[lo_k]= lo_ik;
  t[lo_ik]= lo_k;
 }
 return t;
}();
constexpr u64 embed_idx(u16 idx) {
 static constexpr auto EMBED= []() {
  u64 SUBFIELD_BASIS[]= {0x0000000000000001ull, 0x5fbfaec6aeac0002ull, 0xb06c601895640004ull, 0xb013b5277b7c0008ull, 0xb5ebb915248a0010ull, 0x109bb25b2c600020ull, 0xbf3bd95bd4190040ull, 0x0fc66342279b0080ull, 0xb6418f5e57c50100ull, 0xaa194bd4b83f0200ull, 0x1b5217b4dcc70400ull, 0xbb06fa73867a0800ull, 0x006fd55b23331000ull, 0x4ae8fb39198c2000ull, 0xfbd141b29b4f4000ull, 0x1d9ce1776be78000ull};
  array<array<u64, 256>, 2> t{};
  for(int half= 0; half < 2; ++half)
   for(int i= 0; i < 256; ++i) {
    u64 v= 0;
    for(int b= 0; b < 8; ++b)
     if((i >> b) & 1) v^= SUBFIELD_BASIS[b + half * 8];
    t[half][i]= v;
   }
  return t;
 }();
 return EMBED[0][u8(idx)] ^ EMBED[1][idx >> 8];
}
GNU_TARGET("pclmul,vpclmulqdq") u64 log_g(u64 x) {
 assert(x);
 u64 x_f16, g, x_6700417, x_641;
 const u64 x32= frob32(x);
 const u64 N= mul(x, x32);  // x^(2^32+1) ∈ F_2^32
 const u64 fN= frob16(N);
 // x_f16 = N·frob16(N) = x^M ∈ F_2^16^*、g = frob32(x)·frob16(N) = x^(M-1)
 tie(x_f16, g)= unpack(mul2(_mm256_set_epi64x(0, x32, 0, N), _mm256_set1_epi64x(fN)));
 // x^-1 = (x^M)^-1 · x^(M-1) なので x^(2^32-1) = frob32(x) / x = frob32(x) · x^-1
 const u64 s= mul(mul(embed_idx(INV_LOW[u16(x_f16)]), g), x32);
 u64 s7= frob7(s);
 u64 T2= sq(s7), T3= mul(s7, T2);
 u64 T24= frob3(T3), T48= frob4(T3);
 u64 T51, T72;
 tie(T72, T51)= unpack(mul2(_mm256_set_epi64x(0, T3, 0, T24), _mm256_set1_epi64x(T48)));
 tie(x_641, x_6700417)= unpack(mul2(mul2(_mm256_set_epi64x(0, T2, 0, frob10(T51)), _mm256_set_epi64x(0, T3, 0, mul(T72, T51))), _mm256_set1_epi64x(s)));
 const u16 r1= LN16.t[u16(x_f16)];
 const u32 r0= PerfectHash641::lookup(x_641);
 const u32 r2= log_65537(N, fN);
 const u32 r3= BSGSTable6700417::solve(x_6700417);
 const u16 cur_mod_641= r1 % P_641;
 const u16 diff0= (r0 + P_641 - cur_mod_641) % P_641;
 const u16 t0= (diff0 * INV_65535_641) % P_641;
 const u32 cur_after0_mod_F17= (r1 + MOD_F16 * t0) % P_F17;
 const u32 diff2= (r2 + P_F17 - cur_after0_mod_F17) % P_F17;
 const u32 t2= (diff2 * INV_MOD2_F17) % P_F17;
 const u32 cur_after2_mod_BIG= (r1 + (MOD_F16 * t0) % P_BIG + (MOD_F16_641 * t2) % P_BIG) % P_BIG;
 const u32 diff3= (r3 + P_BIG - cur_after2_mod_BIG) % P_BIG;
 const u32 t3= (u64(diff3) * INV_MOD3_BIG) % P_BIG;
 return u64(r1) + MOD_F16 * t0 + MOD_F16_641 * t2 + MOD_F16_641_F17 * t3;
}
}  // namespace gf2_64_log_pohlig_v47
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& xs) {
  using gf2_64_log_pohlig_v47::log_g;
  vector<u64> ans(xs.size());
  for(size_t i= 0; i < xs.size(); ++i) ans[i]= log_g(xs[i]);
  return ans;
 }
};
