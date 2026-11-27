#pragma once
// 多点評価 (実験版)。f の各点 x_i での値を、部分積木 (葉が X - x_i) と余りの木で求める。
// 余りは poly_divmod_fast (fps::inv の上) で作る。手間は O(M(n) log n) で、M は畳み込みの手間。
// 口: multipoint_eval<Conv>(f, x) -> f(x_i) の列。K は体 (ZMod なら法は素数) で、点は相異なること。
#include <algorithm>
#include <cassert>
#include <vector>
#include "neo/base/core/convolve/convolve.hpp"
#include "neo/base/core/poly/poly_divmod_fast.hpp"

namespace multipoint_internal {
template <class Conv, class K> std::vector<K> mul(const std::vector<K>& a, const std::vector<K>& b) {
 if (a.empty() || b.empty()) return {};
 std::vector<K> c(a.size() + b.size() - 1);
 convolve_range<Conv>(a.data(), int(a.size()), b.data(), int(b.size()), 0, int(c.size()), c.data());
 return c;
}
}  // namespace multipoint_internal

template <class Conv= convolve_internal::Auto, class K>
std::vector<K> multipoint_eval(const std::vector<K>& f, const std::vector<K>& x) {
 using multipoint_internal::mul;
 const int m= int(x.size());
 std::vector<K> res(m);
 if (m == 0) return res;
 if (f.empty()) return res;
 if (f.size() == 1) {
  std::fill(res.begin(), res.end(), f[0]);
  return res;
 }
 int sz= 1;
 while (sz < m) sz<<= 1;
 // 部分積木。点の無い葉は 1 にしておき、積で消える。
 std::vector<std::vector<K>> tr(2 * sz);
 for (int i= 0; i < m; ++i) tr[sz + i]= {-x[i], K(1)};
 for (int i= m; i < sz; ++i) tr[sz + i]= {K(1)};
 for (int i= sz - 1; i >= 1; --i) tr[i]= mul<Conv>(tr[2 * i], tr[2 * i + 1]);
 // 余りの木。葉の余りは f(x_i) (0 次)。
 std::vector<std::vector<K>> rem(2 * sz);
 rem[1]= poly_divmod_fast<Conv>(f, tr[1]).second;
 for (int i= 1; i < sz; ++i) {
  if (tr[2 * i].size() == 1) continue;  // 点の無い節
  rem[2 * i]= poly_divmod_fast<Conv>(rem[i], tr[2 * i]).second;
  rem[2 * i + 1]= poly_divmod_fast<Conv>(rem[i], tr[2 * i + 1]).second;
 }
 for (int i= 0; i < m; ++i) res[i]= rem[sz + i].empty() ? K() : rem[sz + i][0];
 return res;
}
