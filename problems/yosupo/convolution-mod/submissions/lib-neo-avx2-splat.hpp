#pragma once
#include "../common.hpp"
// 実験中の単独の NTT の核 (5 本目、ひねりを全レーン同じにした形 (表が要素あたり 1/8)) を yosupo の口で呼ぶ。
#include "neo/base/core/algebra/ZMod.hpp"
#include "_shared/ntt/ntt_avx2_splat.hpp"
inline std::vector<u32> run(const std::vector<u32>& a, const std::vector<u32>& b) {
 using K= ZMod<998244353>;
 const int n= (int)a.size(), m= (int)b.size();
 if (!n || !m) return {};
 std::vector<K> fa(n), fb(m), fc(n + m - 1);
 for (int i= 0; i < n; ++i) fa[i]= K::raw(a[i]);
 for (int i= 0; i < m; ++i) fb[i]= K::raw(b[i]);
 NttAvx2Splat::convolve_range(fa.data(), n, fb.data(), m, 0, n + m - 1, fc.data());
 std::vector<u32> c(n + m - 1);
 for (int i= 0; i < n + m - 1; ++i) c[i]= fc[i].val();
 return c;
}
