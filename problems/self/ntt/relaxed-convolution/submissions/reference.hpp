#pragma once
// 参照実装: ライブラリを読まない NTT で全体の積を求める (オンラインにはしない)。答えは n + m - 1 個。
#include "_shared/ntt/_reference.hpp"

struct Solver {
 static constexpr unsigned M= 998244353;
 const vector<u32>& f;
 const vector<u32>& g;
 vector<u32> ans;

 Solver(const vector<u32>& f_, const vector<u32>& g_): f(f_), g(g_), ans(f_.size() + g_.size() - 1) {}

 void run() { ntt_reference::convolve_range<M>(f.data(), int(f.size()), g.data(), int(g.size()), 0, int(ans.size()), ans.data()); }

 const vector<u32>& answer() const { return ans; }
};
