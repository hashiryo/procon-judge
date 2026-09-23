#pragma once
// _subfield32.hpp の続き: μ_P 成分の log を s = b^(2^16-1) から引く方 (提出ではない)。
//
// b = u v (u ∈ μ_P, v ∈ μ_N) に対し s = b^N = u^N なので、N' = N^-1 mod P を掛ければ
// log_{h_P}(u) = N' log_{h_P}(s) が戻る。N' は log 表の値に畳んである。
// s は Itoh-Tsujii と同じ形で b^3 → b^15 → b^255 → b^(2^16-1) と 4 回の乗算で作る。
//
// log 表の添字の作り方: s ∈ μ_P は s^(2^16) = s^-1 だから y = s + frob16(s) は F_2^16 の
// 元で、s ↦ y はちょうど 2 対 1 (同じ y を持つのは s と s^-1 だけ)。F_2^16 の元は poly
// 表現の下位 16 bit がそのまま識別子なので、65536 要素の表を y の 16 bit で直接引ける。
// 2 つのどちらかは s と frob16(s) の大小で決め、表には小さい方の log が入っている。
// gf2-64-log の 65537 用ハッシュ表と違って probe も fingerprint も要らない。
#include "_subfield32.hpp"
namespace gf2_64_pow_subfield32 {
using gf2_64_pclmul::frob2;
using gf2_64_pclmul::frob4;
using gf2_64_pclmul::frob8;
// LN[u16(s + s^-1)] = (log_{h_P}(s と s^-1 の小さい方) * N') mod P
struct MuLn {
 u32 LN[65536];
};
constexpr MuLn MU_LN= []() {
 MuLn r{};
 u64 cur= 1;
 u32 v= 0;  // (k * N') mod P
 for(u32 k= 0; k < 65537; ++k) {
  const u64 fr= apply_bt(FROB16_RAW, cur);  // = cur^-1
  if(cur <= fr) r.LN[u16(cur ^ fr)]= v;
  cur= apply_bt(MUL_HP, cur);
  v+= CRT_FOLD;
  if(v >= MOD_P) v-= MOD_P;
 }
 return r;
}();
// b = a^(2^32+1) ∈ F_2^32 と q から、b^q = h_P^mp · h_N^mn となる (mp, mn) を返す。
// そこから元を作るのは pow_pair (_subfield32.hpp) か pow_pair_full (_subfield32_pwfull.hpp)。
GNU_TARGET("pclmul,vpclmulqdq") inline pair<u32, u32> subfield_exp2(u64 b, u32 q) {
 const u64 t= mul(b, frob16(b));  // b^(2^16+1) = b^P
 u64 s= mul(b, sq(b));            // b^3
 s= mul(s, frob2(s));             // b^15
 s= mul(s, frob4(s));             // b^255
 s= mul(s, frob8(s));             // b^(2^16-1) = b^N ∈ μ_P
 const u64 fs= frob16(s);         // = s^-1
 u32 kp= MU_LN.LN[u16(s ^ fs)];
 if(s > fs) kp= MOD_P - kp;  // 表に入っているのは小さい方の log
 return {u32(u64(kp) * q % MOD_P), u32(u64(SIG.LN[u16(t)]) * q % MOD_N)};
}
}  // namespace gf2_64_pow_subfield32
