#pragma once
// 比較の基準。本体は現行の _shared/gf2-64/mul2.hpp と同じで、1 反復で 2 積を出す。
// 各 128bit half の下位 qword どうしを掛け (imm 0x00)、reduce も __m256i のまま 2 lane 並列でやる。
// operand は set_epi64x で 2 本ずつ組み、答えは extract で 2 本に戻す。T が奇数のときの端数だけ
// スカラの mul (_shared/gf2-64/mul.hpp) に落とす。
#pragma GCC optimize("O3,unroll-loops")
#include "_shared/gf2-64/_common.hpp"
#include "_shared/gf2-64/mul.hpp"
namespace gf2_64_mul2_2lane {
const __m256i RED_TABLE= GF2_64_M256_SETR_EPI8(0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 45, 54, 90, 65, 119, 108, 0, 0, 0, 0, 0, 0, 0, 0);
GNU_TARGET("vpclmulqdq") inline __m256i mul2(const __m256i& a_vec, const __m256i& b_vec) {
 __m256i prod= _mm256_clmulepi64_epi128(a_vec, b_vec, 0);
 __m256i h= _mm256_srli_si256(prod, 8);
 return _mm256_xor_si256(_mm256_xor_si256(prod, _mm256_shuffle_epi8(RED_TABLE, _mm256_srli_epi64(h, 60))), _mm256_xor_si256(_mm256_xor_si256(h, _mm256_slli_epi64(h, 1)), _mm256_xor_si256(_mm256_slli_epi64(h, 3), _mm256_slli_epi64(h, 4))));
}
}
struct GF2_64Op {
 GNU_TARGET("pclmul,vpclmulqdq") static vector<u64> run(const vector<u64>& as, const vector<u64>& bs) {
  using gf2_64_mul2_2lane::mul2;
  const size_t n= as.size();
  vector<u64> ans(n);
  size_t i= 0;
  for(; i + 2 <= n; i+= 2) {
   __m256i r= mul2(_mm256_set_epi64x(0, as[i + 1], 0, as[i]), _mm256_set_epi64x(0, bs[i + 1], 0, bs[i]));
   ans[i]= u64(_mm256_extract_epi64(r, 0));
   ans[i + 1]= u64(_mm256_extract_epi64(r, 2));
  }
  if(i < n) ans[i]= gf2_64_pclmul::mul(as[i], bs[i]);
  return ans;
 }
};
