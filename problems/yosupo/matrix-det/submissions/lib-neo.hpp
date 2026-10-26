#pragma once
// NeoLibrary の det (neo/linalg/det.hpp)。この問題で書き比べた zmod_det_blk8 を写したもので、法を問わない形 (単元のない列だけ
// 互除法で消す、yosupo-matrix-det-arbitrary-mod の zmod_det_blk8) にしてある。法が素数なので互除法の道は通らない。
#include "../common.hpp"
#include "neo/linalg/det.hpp"
inline u32 run(int n, const vector<vector<u32>>& a) {
 using Z= ZMod<MOD>;
 Matrix<Z> A(n, n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) A[i][j]= Z::raw(a[i][j]);
 return det(std::move(A)).val();
}
