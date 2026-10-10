#pragma once
// 表を持たない比べる相手。1 回の呼び出しの latency を詰めた Miller-Rabin の核 (_mr_single.hpp) で、
// 2^64 未満で決定的な Jim Sinclair の 7 つの底を全部同時に回す。
// 底は n < 1373653 なら {2, 3}、2^32 未満なら {2, 7, 61}、それ以上は
// {2, 325, 9375, 28178, 450775, 9780504, 1795265022} (https://miller-rabin.appspot.com/)。
// n >= 2^32 では底がどれも n 未満なので、底を n で割る必要が無い。
#include "../common.hpp"
#include "_mr_single.hpp"

inline bool is_prime_sinclair7(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  if (n < 1373653) {
    const u64 b[1] = {3};
    return mr_single::sprp(n, b);
  }
  if (n < (1ull << 32)) {
    const u64 b[2] = {7, 61};
    return mr_single::sprp(n, b);
  }
  const u64 b[6] = {325, 9375, 28178, 450775, 9780504, 1795265022};
  return mr_single::sprp(n, b);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_sinclair7(qs[i]);
  return ans;
}
