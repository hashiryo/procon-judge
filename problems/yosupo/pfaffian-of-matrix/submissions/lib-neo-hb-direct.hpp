#pragma once
// 確かめ用。_neo_pf_hb.hpp の写しに入力の vector<vector<u32>> を直接読ませる (Matrix<Z> を作らない)。lib-neo-hb との差は
// Matrix<Z> (n = 1000 で 4 MB) を作って写す分で、zmod_pf_hpv との差は写しと手本の書き方の違い。
#include "_neo_pf_hb.hpp"
inline u32 run(int N, const vector<vector<u32>>& M_in) {
 using Z= ZMod<MOD>;
 const int n= 2 * N;
 return neo_pf_hb::zmod_v<Z>(n, [&](int i, int j) { return M_in[i][j]; }).val();
}
