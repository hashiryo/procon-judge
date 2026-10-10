#pragma once
// fj64_single.hpp と同じ FJ64_262K の 2 底の判定で、底 2 の鎖の 2 倍を還元の中に畳んだもの (_mr_single.hpp の sprp_fold)。
// fj64_single では「片方の因数を 2 倍してから掛ける」シフトが鎖に入り、底 2 の鎖が 12 サイクル、表の底の鎖が 11 サイクルだった。
// 畳むと底 2 の鎖も 11 サイクルになる見込み。
#include "../common.hpp"
#include "_mr_single.hpp"
#include "_fj64_262k.hpp"

inline bool is_prime_fj64_fold(u64 n) {
  if (int c = mr_single::small_check(n); c >= 0) return c;
  if (n < 1373653) {
    const u64 b[1] = {3};
    return mr_single::sprp_fold(n, b);
  }
  const u64 b[1] = {fj64_bases[fj64_hash(n)]};
  return mr_single::sprp_fold(n, b);
}

inline vector<bool> run(const vector<u64>& qs) {
  vector<bool> ans(qs.size());
  for (size_t i = 0; i < qs.size(); ++i) ans[i] = is_prime_fj64_fold(qs[i]);
  return ans;
}
