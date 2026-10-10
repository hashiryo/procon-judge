#pragma once
// 1 回の呼び出しの latency を詰めた Miller-Rabin の核 (_mr_single.hpp) に、FJ64_262K の表を載せたもの。
// 底は n < 1373653 なら {2, 3}、それ以上は {2, 表の底} の 2 つで、2 本の鎖を同時に回す。
// yosupo_fastest.hpp (rogi52 さんの提出 365922) と同じ表で、違いは核だけ。
#include "../common.hpp"
#include "_mr_single.hpp"
#include "_fj64_262k.hpp"

inline bool is_prime_fj64(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  if (n < 1373653) {
    const u64 b[1] = {3};
    return mr_single::sprp(n, b);
  }
  const u64 b[1] = {fj64_bases[fj64_hash(n)]};
  return mr_single::sprp(n, b);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_fj64(qs[i]);
  return ans;
}
