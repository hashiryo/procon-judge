#pragma once
// CRT の核の AVX2 版。3 素数の単独の NTT を 3 本目 (AVX2、Montgomery、基数 4) で回す。Garner と還元は crt.hpp と同じ。
#include "_shared/ntt/crt.hpp"
#include "_shared/ntt/ntt_avx2.hpp"

using CrtAvx2= crt_kernel<NttAvx2>;
