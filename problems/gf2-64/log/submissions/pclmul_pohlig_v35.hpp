#pragma once
// v34_3 の 65537 用の表を、ハッシュ表から constexpr の直接引き表に替えた版。
//
// 位数 65537 の部分群の元は s^(2^16) = s^-1 なので、y = s + frob16(s) は F_2^16 の元に
// なり、s ↦ y はちょうど 2 対 1 (同じ y を持つのは s と s^-1 だけ)。F_2^16 の元は poly
// 表現の下位 16 bit がそのまま識別子なので、65536 要素の表を y の 16 bit で直接引ける。
// 2 つのどちらかは s と frob16(s) の大小で決まる。
//
// これで DirectLogTable_65537 (2 MB の packed linear-probing、実行時構築) が 256 KB の
// constexpr 表になる。probe も fingerprint も要らず、プロセスごとの 65537 回の乗算と
// 2 MB の memset も消える。gf2-64-log はケースが 5 つでクエリが 1 万程度なので、
// 表作りの時間が記録に効いている。
//
// それ以外 (641 の完全ハッシュ、6700417 の BSGS、CRT の組み方) は v34_3 と同じ。
//
// 必要な拡張: VPCLMULQDQ + AVX2 (Intel Ice Lake / AMD Zen3 以降).
#pragma GCC optimize("O3,unroll-loops")
#define GF2_64_EXTRA_TARGETS "vpclmulqdq"
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
#include "_shared/gf2-64/mul2.hpp"
#include "_shared/gf2-64/sq.hpp"
#include "_shared/gf2-64/frob.hpp"
namespace gf2_64_log_pohlig_v35 {
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
// pclmul intrinsic は constexpr 化できないため、4-bit windowed CLMUL で実装。
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
constexpr auto LN16= []() {
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
 array<u16, 65536> ln{};
 u16 cur= 1;
 for(u32 k= 0; k < 65535; ++k) {
  u16 lo= T_lo[u8(cur)] ^ T_hi[cur >> 8];
  ln[lo]= u32(u16(k)) * 2699 % 65535;
  cur= u16(cur << 1) ^ (0x002DU & -u16(cur >> 15));
 }
 ln[0]= 0;
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
// LogTable65537: 65537 元 subgroup 用 直接引き表 (constexpr, 256 KB).
//
// s^(2^16) = s^-1 なので y = s + frob16(s) ∈ F_2^16 で、s ↦ y は 2 対 1。F_2^16 の元は
// poly 表現の下位 16 bit がそのまま識別子 (部分体の基底の取り方) なので、y の 16 bit を
// 添字にできる。表には s と s^-1 のうち小さい方の log を入れてあり、大きい方なら
// 65537 - (表の値) が log になる。
//
// 表は生配列で持つこと。65537 回の walk を std::array の operator[] で回すと clang の
// constexpr step 上限に当たる。
// =============================================================================
struct RawByteTable {
 u64 t[8][256];
};
constexpr u64 apply_raw(const RawByteTable &T, u64 a) { return T.t[0][u8(a)] ^ T.t[1][u8(a >> 8)] ^ T.t[2][u8(a >> 16)] ^ T.t[3][u8(a >> 24)] ^ T.t[4][u8(a >> 32)] ^ T.t[5][u8(a >> 40)] ^ T.t[6][u8(a >> 48)] ^ T.t[7][u8(a >> 56)]; }
// G_65537 倍。基底の像は x 倍 (shift + reduce) を 64 回繰り返すだけで出る。
constexpr RawByteTable MUL_G17= []() {
 u64 basis[64]{};
 basis[0]= G_65537;
 for(int i= 1; i < 64; ++i) basis[i]= (basis[i - 1] << 1) ^ (IRRED_LOW & -(basis[i - 1] >> 63));
 RawByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int j= 0; j < 8; ++j) {
   const u64 v= basis[8 * p + j];
   for(int b= 0; b < (1 << j); ++b) r.t[p][(1 << j) | b]= r.t[p][b] ^ v;
  }
 return r;
}();
// frob.hpp の表は std::array なので、walk で引く用に生配列へ写しておく。
constexpr RawByteTable FROB16_RAW= []() {
 RawByteTable r{};
 for(int p= 0; p < 8; ++p)
  for(int b= 0; b < 256; ++b) r.t[p][b]= gf2_64_pclmul::FROB16_BYTE[p][b];
 return r;
}();
struct LogTable65537 {
 u32 t[65536];
};
constexpr LogTable65537 LOG65537= []() {
 LogTable65537 r{};
 u64 cur= 1;
 for(u32 k= 0; k < P_F17; ++k) {
  const u64 fr= apply_raw(FROB16_RAW, cur);  // = cur^-1
  if(cur <= fr) r.t[u16(cur ^ fr)]= k;
  cur= apply_raw(MUL_G17, cur);
 }
 return r;
}();
inline u32 log_65537(u64 s) {
 const u64 fs= frob16(s);
 const u32 v= LOG65537.t[u16(s ^ fs)];
 return s > fs ? u32(P_F17) - v : v;
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
 static constexpr u64 EMPTY= ~u64(0);
 static constexpr u64 inv_base_m= 0x1489880b9cf723deull;
 static constexpr u64 inv2_base_m= 0x5be693c8c2c557e3ull;            // = inv_base_m^2
 static constexpr u64 inv3_base_m= mul_ce(inv_base_m, inv2_base_m);  // = inv_base_m^3
 static constexpr u64 inv4_base_m= 0xfdb44dcbca6522deull;            // = inv_base_m^4  (4-streams 用)
 std::vector<u64> tab;
 void build() {
  u64 base= G_6700417;
  tab.assign(mask + 1, EMPTY);
  u64 cur= 1;
  for(u32 j= 0; j < m; ++j) {
   u32 h= u32(cur) & mask;
   while(tab[h] != EMPTY) h= (h + 1) & mask;
   tab[h]= (cur & HI_MASK) | u64(j);
   cur= mul(cur, base);
  }
 }
 u32 solve(u64 target) const {
  u64 t[4], t_n[4], t_n2[4];
  // Initial t[0..3] = target * inv^k (k=0..3).
  // mul2: lane0 -> first scalar, lane2 -> second scalar.
  t[0]= target;
  // (t[1], t[2]) = (target*inv, target*inv2)  -- 2 並列
  tie(t[1], t[2])= unpack(mul2(_mm256_set1_epi64x(target), _mm256_set_epi64x(0, inv2_base_m, 0, inv_base_m)));
  // (t[3], t_n[0]) = (target*inv3, target*inv4)  -- 2 並列
  tie(t[3], t_n[0])= unpack(mul2(_mm256_set1_epi64x(target), _mm256_set_epi64x(0, inv4_base_m, 0, inv3_base_m)));
  // t_n[1..3] = t[1..3] * inv4  ->  pair (t_n[1], t_n[2]) と (t_n[3], t_n2[0]) で並列化
  __m256i inv4_v= _mm256_set1_epi64x(inv4_base_m);
  tie(t_n[1], t_n[2])= unpack(mul2(_mm256_set_epi64x(0, t[2], 0, t[1]), inv4_v));
  tie(t_n[3], t_n2[0])= unpack(mul2(_mm256_set_epi64x(0, t_n[0], 0, t[3]), inv4_v));
  // t_n2[1..3] = t_n[1..3] * inv4  ->  mul2 x2 (奇数分は無駄 lane なし)
  tie(t_n2[1], t_n2[2])= unpack(mul2(_mm256_set_epi64x(0, t_n[2], 0, t_n[1]), inv4_v));
  // 残り1個 (t_n2[3]) は scalar mul で済ます
  t_n2[3]= mul(t_n[3], inv4_base_m);
  for(int j= 0; j < 4; ++j) {
   _mm_prefetch((const char*)&tab[u32(t[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&tab[u32(t_n[j]) & mask], _MM_HINT_T0);
   _mm_prefetch((const char*)&tab[u32(t_n2[j]) & mask], _MM_HINT_T0);
  }
  for(u8 i= 0; i <= max_i; i+= 4) {
   for(int j= 0; j < 4; ++j) {
    if(i + j > max_i) break;
    u64 tt= t[j];
    u32 h= u32(tt) & mask;
    while(tab[h] != EMPTY) {
     u64 e= tab[h];
     if(((e ^ tt) & HI_MASK) == 0) return u32((i + j) * m + u32(e & mask));
     h= (h + 1) & mask;
    }
   }
   for(int j= 0; j < 4; ++j) {
    t[j]= t_n[j];
    t_n[j]= t_n2[j];
   }
   if(i + 12 <= max_i) {
    // t_n2[0..3] = t_n[0..3] * inv4  -- mul2 x2 で 4 並列
    tie(t_n2[0], t_n2[1])= unpack(mul2(_mm256_set_epi64x(0, t_n[1], 0, t_n[0]), inv4_v));
    tie(t_n2[2], t_n2[3])= unpack(mul2(_mm256_set_epi64x(0, t_n[3], 0, t_n[2]), inv4_v));
    for(int j= 0; j < 4; ++j) _mm_prefetch((const char*)&tab[u32(t_n2[j]) & mask], _MM_HINT_T0);
   }
  }
  return q;
 }
};
inline BSGSTable6700417 bsgs_6700417;

inline bool inited= false;
void init_tables() {
 if(inited) return;
 inited= true;
 // PerfectHash641 と LOG65537 は constexpr で .rodata に焼かれているため build 不要。
 bsgs_6700417.build();
}
u64 log_g(u64 x) {
 assert(x);
 u64 N, s, x_f16, x_65537, x_6700417, x_641;
 __m256i Ns= mul2(_mm256_set_epi64x(0, sq(x), 0, frob32(x)), _mm256_set1_epi64x(x));
 tie(N, s)= unpack(Ns);
 tie(x_f16, s)= unpack(mul2(_mm256_set_epi64x(0, frob2(s), 0, frob16(N)), Ns));
 s= mul(s, frob4(s));
 s= mul(s, frob8(s));  // 2^16-1
 tie(x_65537, s)= unpack(mul2(_mm256_set_epi64x(0, frob16(s), 0, frob32(s)), _mm256_set1_epi64x(s)));
 u64 s7= frob7(s);
 u64 T2= sq(s7), T3= mul(s7, T2);
 u64 T24= frob3(T3), T48= frob4(T3);
 u64 T51, T72;
 tie(T72, T51)= unpack(mul2(_mm256_set_epi64x(0, T3, 0, T24), _mm256_set1_epi64x(T48)));
 tie(x_641, x_6700417)= unpack(mul2(mul2(_mm256_set_epi64x(0, T2, 0, frob10(T51)), _mm256_set_epi64x(0, T3, 0, mul(T72, T51))), _mm256_set1_epi64x(s)));
 const u16 r1= LN16[u16(x_f16)];
 const u32 r0= PerfectHash641::lookup(x_641);
 const u32 r2= log_65537(x_65537);
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
}  // namespace gf2_64_log_pohlig_v35
struct GF2_64Op {
 static vector<u64> run(const vector<u64>& xs) {
  using gf2_64_log_pohlig_v35::init_tables;
  using gf2_64_log_pohlig_v35::log_g;
  init_tables();
  vector<u64> ans(xs.size());
  for(size_t i= 0; i < xs.size(); ++i) ans[i]= log_g(xs[i]);
  return ans;
 }
};
