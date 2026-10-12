#pragma once
// 同じ値が最後に出たときの DP の値を、NeoLibrary の HashMap (開番地法) に値ごとに持つ版。dp[i + 1] = 2 dp[i] - (a[i] が前に出たときの
// dp) で、表には dp[i] を入れ直す。DP の列と前の位置の列を持たずに 1 回の走査で済む。表の値は dp + 1 にして、0 を「まだ出ていない」
// にする (HashMap は 0 の bit 列を無い値として扱う)。
// 異なる部分列の数え上げの実装を比べた記録は algo-notes の notes/number-of-subsequences.md にある。
#include <vector>
#include "pj.hpp"
#include "neo/data_structure/HashMap.hpp"

struct Solver {
  vector<unsigned> a;
  unsigned ans = 0;

  explicit Solver(const vector<unsigned> &a) : a(a) {}

  void run() {
    constexpr unsigned P = 998244353;
    HashMap<unsigned> seen;
    unsigned d = 1;
    for (const unsigned x : a) {
      unsigned &r = seen[x];
      const unsigned before = d;
      d *= 2;
      d -= d >= P ? P : 0;
      if (r) {
        d += P - (r - 1);
        d -= d >= P ? P : 0;
      }
      r = before + 1;
    }
    ans = d == 0 ? P - 1 : d - 1;
  }

  unsigned answer() const { return ans; }
};
