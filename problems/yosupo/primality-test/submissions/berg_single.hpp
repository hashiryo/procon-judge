#pragma once
// 1 回の呼び出しの latency を詰めた Miller-Rabin の核 (_mr_single.hpp) に、Bradley Berg の表を載せたもの。
// 底は n < 1373653 なら {2, 3}、2^32 未満なら {2, 7, 61}、2^49 未満なら {2, 表の底}、
// それ以上は {2, 表の底, 3 つ目の底} で、全部の鎖を同時に回す。
// 表は 16384 個 (32KB) で、FJ64_262K (262144 個、512KB) より小さい代わりに 2^49 以上で底が 1 つ多い。
#include "../common.hpp"
#include "_mr_single.hpp"
#include "_berg64_16k.hpp"

inline bool is_prime_berg(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  if (n < 1373653) {
    const u64 b[1] = {3};
    return mr_single::sprp(n, b);
  }
  if (n < (1ull << 32)) {
    const u64 b[2] = {7, 61};
    return mr_single::sprp(n, b);
  }
  const u64 b0 = berg64_bases[berg64_hash(n)];
  if (n < (1ull << 49)) {
    const u64 b[1] = {b0};
    return mr_single::sprp(n, b);
  }
  const u64 b[2] = {b0, berg64_third[b0 >> 13]};
  return mr_single::sprp(n, b);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_berg(qs[i]);
  return ans;
}
