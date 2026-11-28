#pragma once
// CRT の核の AVX2 版で、中の単独の NTT を 5 本目 (splat) にしたもの。3 素数の単独の NTT を splat で回す。
// Garner と還元は crt.hpp と同じ。
#include "_shared/ntt/crt.hpp"
#include "_shared/ntt/ntt_avx2_splat.hpp"

using CrtAvx2Splat= crt_kernel<NttAvx2Splat>;
