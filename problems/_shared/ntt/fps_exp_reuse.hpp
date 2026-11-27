#pragma once
// 実験版の exp (1/g を持ち回る形)。
// NeoLibrary の fps::exp は段ごとに log g を作るので、その中で 1/g を毎回 Newton 法で作り直し、
// 変換の長さが 4m になります。ここでは 1/g を段の中で 1 回だけ直して持ち回り、変換を長さ 2m に抑えます。
// 口は fps::exp と同じ (f[0] = 0、先頭 n 項) で、畳み込みの核はテンプレート引数で受けます。
#include <algorithm>
#include <cassert>
#include <vector>
#include "neo/base/core/convolve/convolve.hpp"

namespace fps_exp_reuse_internal {
// f の先頭 n 項の exp。f[0] = 0。
// 段の入口で g = exp f の x^m まで、h = 1/g の x^m までが揃っているとして、m2 = min(2m, n) 桁へ:
//   1. h1 = h (2 - g h) の x^{m2} まで (g は x^m までの切り詰めの逆数。誤差が 2 乗になって 2m 桁に届く)
//   2. c = (log g)' = g' / g の [m-1, m2-1) から w = f - log g の [m, m2)
//   3. g の x^m 以上 <- (g w) の先頭 cnt 項 (w は x^m 以上だけを持つ)
//   4. h の x^m 以上 <- h1 - h1 w の x^m 以上 (g が x^m 以上で変わった分を 1 次の項で直す)
template <class Conv, class K> std::vector<K> exp_reuse(const std::vector<K>& f, int n) {
 assert(n >= 1);
 assert(f.empty() || f[0] == K());
 std::vector<K> g(1, K(1)), h(1, K(1));
 for (int m= 1; m < n;) {
  const int m2= std::min(2 * m, n), cnt= m2 - m;
  // 1. 1/g の Newton 1 段。
  std::vector<K> h1(m2), t(cnt), hh(cnt);
  convolve_range<Conv>(g.data(), m, h.data(), m, m, m2, t.data());  // (g h) の [m, m2)
  for (int i= 0; i < m; ++i) h1[i]= h[i];
  for (int i= 0; i < cnt; ++i) t[i]= -t[i];                        // 1 - g h の [m, m2)
  convolve_range<Conv>(h.data(), m, t.data(), cnt, 0, cnt, hh.data());
  for (int i= 0; i < cnt; ++i) h1[m + i]= hh[i];
  // 2. (log g)' の [m-1, m2-1) を積分して w = f - log g の [m, m2)。
  std::vector<K> c(cnt), w(cnt);
  if (m >= 2) {
   std::vector<K> dg(m - 1);
   for (int i= 0; i < m - 1; ++i) dg[i]= g[i + 1] * K(i + 1);
   convolve_range<Conv>(dg.data(), m - 1, h1.data(), m2, m - 1, m2 - 1, c.data());
  }
  for (int i= m; i < m2; ++i) w[i - m]= (i < int(f.size()) ? f[i] : K()) - c[i - m] / K(i);
  // 3. g の x^m 以上。
  std::vector<K> gg(cnt), hw(cnt);
  convolve_range<Conv>(g.data(), m, w.data(), cnt, 0, cnt, gg.data());
  // 4. h の x^m 以上。1/(g + g w) = h1 - h1 w を x^{m2} まで。
  convolve_range<Conv>(h1.data(), m, w.data(), cnt, 0, cnt, hw.data());
  g.resize(m2), h.resize(m2);
  for (int i= 0; i < cnt; ++i) g[m + i]= gg[i], h[m + i]= h1[m + i] - hw[i];
  m= m2;
 }
 return g;
}
}  // namespace fps_exp_reuse_internal

template <class Conv= convolve_internal::Auto, class K> std::vector<K> fps_exp_reuse(const std::vector<K>& f, int n) {
 return fps_exp_reuse_internal::exp_reuse<Conv, K>(f, n);
}
