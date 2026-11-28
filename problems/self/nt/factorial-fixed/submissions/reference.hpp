// 期待出力を作る参照実装。今の Library の FactorialPrecalculation (iv、fct、fiv の 3 本) と同じ算法を、
// ライブラリを include しない形に書き出したものです。法は 998244353 固定、掛け算は 64 bit で取ります。
#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

namespace reference_impl {
using u64 = unsigned long long;

constexpr u64 MOD = 998244353;

// n! までの fct、fiv と、k の逆元 iv。iv[k] は、q = ceil(MOD / k) として iv[k q - MOD] * q。
// 値は MOD 未満なので unsigned で持つ (N = 1e7 で 3 本合わせて 120 MB)。
struct Tables {
  std::vector<unsigned> iv, fct, fiv;
  explicit Tables(int n) : iv(n + 1, 1), fct(n + 1, 1), fiv(n + 1, 1) {
    for (u64 k = 2; k <= (u64)n; ++k) {
      const u64 q = (MOD + k - 1) / k;
      iv[k] = (unsigned)((u64)iv[k * q - MOD] * q % MOD);
    }
    for (int k = 1; k <= n; ++k) fct[k] = (unsigned)((u64)fct[k - 1] * k % MOD);
    for (int k = 1; k <= n; ++k) fiv[k] = (unsigned)((u64)fiv[k - 1] * iv[k] % MOD);
  }
};
}  // namespace reference_impl

struct Solver {
  std::vector<unsigned> run(unsigned N, const std::vector<unsigned>& ns, const std::vector<unsigned>& rs) {
    (void)N;
    int mx = 0;
    for (unsigned n : ns) mx = std::max(mx, (int)n);
    const reference_impl::Tables t(mx);
    std::vector<unsigned> out;
    out.reserve(ns.size());
    for (std::size_t i = 0; i < ns.size(); ++i) {
      const int n = (int)ns[i], r = (int)rs[i];
      out.push_back((unsigned)((reference_impl::u64)t.fct[n] * t.fiv[r] % reference_impl::MOD * t.fiv[n - r] % reference_impl::MOD));
    }
    return out;
  }
};
