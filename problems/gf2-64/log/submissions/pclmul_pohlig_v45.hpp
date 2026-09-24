#pragma once
// v39 (giant step から frob32 を追い出し、z と z^-1 を 2 本の等比数列で持つ版) に、
// v42 の変更を載せた版。
//
// v42 の変更は、定数のベクタを関数の外の static にすることと、mul2 の出力をそのまま
// 次の mul2 へ渡して unpack と詰め直しを省くことの 2 つ。v39 とは相性が良くて、
// (z, w) の組を 1 本のベクタに載せておくと、進めるのは mul2(P, (g^-2m, g^+2m)) の
// 1 本で済み、出力がそのまま次の入力の形になる。
//
// x64 では v39 が gcc の 1 位 (4.88 ms) だったので、その上に v42 の変更を足した形。
//
// 必要な拡張: VPCLMULQDQ + AVX2 (Intel Ice Lake / AMD Zen3 以降).
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_log_pohlig_v45 {
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
// x^(2^32-1) を作ってから y = x + x^-1 を添字にする代わりに、1 行目で出ている
// N = x^(2^32+1) (F_2^32 の元) の座標の比を添字にする。
//
// φ を φ + frob16(φ) = 1 に取ると N = b_0 + φ b_1 (b_0, b_1 ∈ F_2^16) と分かれて
//   b_1 = N + frob16(N)   … frob16(N) は x_f16 のために既に計算してある
//   b_0 = N + φ b_1       … φ 倍は 1 KB の表を 2 回
// で座標が取れる。N に F_2^16 の元を掛けても μ_P 成分 u は変わらないので、u は比
// b_0 : b_1 だけで決まる。比の log (F_2^16 の log 表の差) をそのまま添字にする。
//
// 欲しいのは x_65537 = N^65535 = u^-2 の log なので、表には (-2 log u) mod 65537 を
// 入れてある。これで x_65537 を作る mul と frob32、y を作る frob16 が消える代わりに、
// F_2^16 の log 表を 2 回余計に引く。
//
// b_1 = 0 (N ∈ F_2^16 で u = 1) と b_0 = 0 の 2 つの類だけ比が作れないので別扱い。
//
// 表は生配列で持つこと。65537 回の walk を std::array の operator[] で回すと clang の
// constexpr step 上限に当たる。
// =============================================================================
struct RawByteTable {
 u64 t[8][256];
};
constexpr u64 apply_raw(const RawByteTable &T, u64 a) { return T.t[0][u8(a)] ^ T.t[1][u8(a >> 8)] ^ T.t[2][u8(a >> 16)] ^ T.t[3][u8(a >> 24)] ^ T.t[4][u8(a >> 32)] ^ T.t[5][u8(a >> 40)] ^ T.t[6][u8(a >> 48)] ^ T.t[7][u8(a >> 56)]; }
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
constexpr RawByteTable FROB32_RAW= []() {
 RawByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int b= 0; b < 256; ++b) r.t[p][b]= gf2_64_pclmul::FROB32_BYTE[p][b];
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
// BSGSTable6700417: 6700417 元 subgroup 用 BSGS (± の対称性を使う版).
//
// この部分群の位数は 2^32+1 を割るので x^(2^32) = x^-1、つまり frob32 が逆元になる。
// baby step の鍵を κ(z) = z + frob32(z) にすると z と z^-1 が同じ鍵に落ちるので、
// 1 回の giant step が i·2m ± j の 2m 個ぶんを覆う。giant step の刻みを g^(-2m) にして
// baby step を j = 0..m の m+1 個にすると、m = 65536 でも探索は 52 回で足りる
// (v34_3 は m = 131072 で 53 回だった)。
//
// レイアウト: u64 entry = (κ & FP_MASK) | ((j << 1) | flag) << 1
//   - bit 1..18: (j << 1) | flag。flag は「g^j が対 {g^j, g^-j} の大きい方か」
//     (j ≤ m = 65536 なので 17 bit + 1 bit でちょうど収まる)
//   - bit 0 と bit 19..63: κ の fingerprint (FP_MASK)
// 引いた側も z と frob32(z) の大小が分かるので、flag と一致すれば z = g^j で +j、
// 違えば z = g^-j で -j になる。
//
// slot に使うのを下位 18 bit ではなく bit 1..18 にしているのは、鍵が F_2^32 に載って
// いるせい。κ = z + z^-1 は frob32 で固定されるので 32 次元の部分空間の中を動き、その
// 部分空間は 1 を含む。下位 18 bit を slot にすると κ と κ^1 が同じ fingerprint を持った
// まま隣り合う slot に入るので、linear probing が偽の一致を拾ってしまう (実際 1 万件に
// 1 件くらい誤答した)。bit 1..18 を slot にすると、この部分空間と slot の空間の交わりが
// 0 になるので、fingerprint が一致する相手は自分自身しかいない。
//
// 刻みが倍になったぶん、パイプラインの 4 本の間隔も g^(-8m) になる。
// =============================================================================
struct BSGSTable6700417 {
 static constexpr u32 mask= 262143;  // 18-bit, cap = 262144
 static constexpr u64 q= P_BIG;
 static constexpr u32 m= 65536;
 static constexpr u64 gstep= 2 * u64(m);
 static constexpr u64 max_i= 51;
 static constexpr u64 FP_MASK= ~(u64(mask) << 1);  // bit 0 と bit 19..63
 static constexpr u64 step_1= 0x1489880b9cf723deull;   // g^(-2m)
 static constexpr u64 step_2= 0x5be693c8c2c557e3ull;   // g^(-4m)
 static constexpr u64 step_3= mul_ce(step_1, step_2);  // g^(-6m)
 static constexpr u64 step_4= mul_ce(step_2, step_2);  // g^(-8m)
 // 逆元側の刻み。この部分群では frob32 が逆元なので、そのまま反転すれば出る。
 static constexpr u64 up_1= apply_raw(FROB32_RAW, step_1);  // g^(+2m)
 static constexpr u64 up_2= apply_raw(FROB32_RAW, step_2);  // g^(+4m)
 static constexpr u64 up_3= apply_raw(FROB32_RAW, step_3);  // g^(+6m)
 static constexpr u64 up_4= apply_raw(FROB32_RAW, step_4);  // g^(+8m)
 // 空きは 0。bit 1..18 に入れる値を (j + 1) << 1 | flag にして、実在の entry が
 // 0 にならないようにしてある (こうすると表の初期化ループが要らない)。
 struct Tab {
  u64 t[mask + 1];
 };
 static constexpr Tab TAB= []() {
  const RawByteTable mg= make_mul_table(G_6700417);
  Tab r{};
  u64 cur= 1;
  for(u32 j= 0; j <= m; ++j) {
   const u64 fr= apply_raw(FROB32_RAW, cur), key= cur ^ fr;  // fr = cur^-1
   u32 h= u32(key >> 1) & mask;
   while(r.t[h]) h= (h + 1) & mask;
   r.t[h]= (key & FP_MASK) | ((((u64(j) + 1) << 1) | u64(cur > fr)) << 1);
   cur= apply_raw(mg, cur);
  }
  return r;
 }();
 // 定数のベクタは 1 度だけ作る。lane0 が z 側 (g^-2mk)、lane1 が z^-1 側 (g^+2mk)。
 static inline const __m256i V_S1= GF2_64_M256_SET_EPI64X(0, up_1, 0, step_1);
 static inline const __m256i V_S2= GF2_64_M256_SET_EPI64X(0, up_2, 0, step_2);
 static inline const __m256i V_S3= GF2_64_M256_SET_EPI64X(0, up_3, 0, step_3);
 static inline const __m256i V_S4= GF2_64_M256_SET_EPI64X(0, up_4, 0, step_4);
 u32 solve(u64 target) const {
  const u64 iv= frob32(target);  // = target^-1。以降 frob32 は呼ばない
  // (z, w) の組をベクタ 1 本で持つ。mul2 の出力がそのまま次の入力の形なので詰め直しが要らない。
  __m256i P[4], Pn[4];
  P[0]= _mm256_set_epi64x(0, iv, 0, target);
  P[1]= mul2(P[0], V_S1), P[2]= mul2(P[0], V_S2), P[3]= mul2(P[0], V_S3);
  for(int j= 0; j < 4; ++j) Pn[j]= mul2(P[j], V_S4);
  u64 z[4], w[4], zn[4], wn[4];
  for(int j= 0; j < 4; ++j) {
   tie(z[j], w[j])= unpack(P[j]);
   tie(zn[j], wn[j])= unpack(Pn[j]);
   _mm_prefetch((const char *)&TAB.t[u32((z[j] ^ w[j]) >> 1) & mask], _MM_HINT_T0);
   _mm_prefetch((const char *)&TAB.t[u32((zn[j] ^ wn[j]) >> 1) & mask], _MM_HINT_T0);
  }
  for(u8 i= 0; i <= max_i; i+= 4) {
   for(int j= 0; j < 4; ++j) {
    if(i + j > max_i) break;
    const u64 tt= z[j], iw= w[j], kk= tt ^ iw;
    u32 h= u32(kk >> 1) & mask;
    while(TAB.t[h]) {
     const u64 e= TAB.t[h];
     if(((e ^ kk) & FP_MASK) == 0) {
      const u64 val= (e >> 1) & mask, jj= (val >> 1) - 1;
      u64 n= u64(i + j) * gstep;
      if(n >= q) n-= q;
      n+= (u64(tt > iw) == (val & 1)) ? jj : q - jj;
      return u32(n >= q ? n - q : n);
     }
     h= (h + 1) & mask;
    }
   }
   for(int j= 0; j < 4; ++j) P[j]= Pn[j], z[j]= zn[j], w[j]= wn[j];
   if(i + 8 <= max_i)
    for(int j= 0; j < 4; ++j) {
     Pn[j]= mul2(P[j], V_S4);
     tie(zn[j], wn[j])= unpack(Pn[j]);
     _mm_prefetch((const char *)&TAB.t[u32((zn[j] ^ wn[j]) >> 1) & mask], _MM_HINT_T0);
    }
  }
  return q;
 }
};
inline BSGSTable6700417 bsgs_6700417;

// 表は 641 も 65537 も 6700417 も全部 constexpr なので、実行時の構築は無い。
u64 log_g(u64 x) {
 assert(x);
 u64 N, s, x_f16, x_6700417, x_641;
 __m256i Ns= mul2(_mm256_set_epi64x(0, sq(x), 0, frob32(x)), _mm256_set1_epi64x(x));
 tie(N, s)= unpack(Ns);
 const u64 fN= frob16(N);
 tie(x_f16, s)= unpack(mul2(_mm256_set_epi64x(0, frob2(s), 0, fN), Ns));
 s= mul(s, frob4(s));
 s= mul(s, frob8(s));  // 2^16-1
 s= mul(s, frob16(s));  // s^(2^16+1) = x^(2^32-1)。641 と 6700417 の側へ
 u64 s7= frob7(s);
 u64 T2= sq(s7), T3= mul(s7, T2);
 u64 T24= frob3(T3), T48= frob4(T3);
 u64 T51, T72;
 tie(T72, T51)= unpack(mul2(_mm256_set_epi64x(0, T3, 0, T24), _mm256_set1_epi64x(T48)));
 tie(x_641, x_6700417)= unpack(mul2(mul2(_mm256_set_epi64x(0, T2, 0, frob10(T51)), _mm256_set_epi64x(0, T3, 0, mul(T72, T51))), _mm256_set1_epi64x(s)));
 const u16 r1= LN16.t[u16(x_f16)];
 const u32 r0= PerfectHash641::lookup(x_641);
 const u32 r2= log_65537(N, fN);
 const u32 r3= bsgs_6700417.solve(x_6700417);
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
}  // namespace gf2_64_log_pohlig_v45
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& xs) {
  using gf2_64_log_pohlig_v45::log_g;
  vector<u64> ans(xs.size());
  for(size_t i= 0; i < xs.size(); ++i) ans[i]= log_g(xs[i]);
  return ans;
 }
};
