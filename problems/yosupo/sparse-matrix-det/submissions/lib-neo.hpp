#pragma once
// NeoLibrary の sparse_det (neo/linalg/sparse_det.hpp)。この問題で書き比べた zmod_wiedemann_csr を写したもので、乱択がはずれた
// とき (次数が N 未満で定数項が 0 でないとき) に選び直す分が加わる。
#include "../common.hpp"
#include "neo/linalg/sparse_det.hpp"
inline u32 run(int N, int K, const vector<int>& a, const vector<int>& b, const vector<u32>& c) {
 using Z= ZMod<MOD>;
 vector<tuple<int, int, Z>> es(K);
 for(int i= 0; i < K; ++i) es[i]= {a[i], b[i], Z::raw(c[i])};
 return sparse_det(N, es).val();
}
