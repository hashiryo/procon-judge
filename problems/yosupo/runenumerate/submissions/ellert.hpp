#pragma once
// Ellert と Fischer (Linear Time Runs over General Ordered Alphabets、ICALP 2021) の実装 (https://github.com/jonas-ellert/linear-time-runs、
// MIT License) を、目安として呼ぶ提出。接尾辞配列を作らず、2 つの順序それぞれで、Lyndon 配列 (次に小さい接尾辞) と根を後ろへ伸ばす
// LCE を、左から右への 1 回の走査でスタックを使って求める。周期的な区間では、前の位置で求めた辺を写して比べを省く。手前へ伸ばす
// LCE は右から左への走査で、同じように写して求める。比べる文字の数は全体で O(n)。065dfae (2021-06-23) の runs.hpp、LICENSE、AUTHORS
// を、手を入れずに problems/_shared/linear_time_runs/ に置いた。この実装は文字列の両端に、どの文字よりも小さい番兵を求めるので、
// 前後に '\0' を付けた写しを渡す。返る (始まり, 周期, 長さ) の列は始まりと周期の昇順なので、周期で数え分けて (周期, 始まり, 終わり)
// の昇順にする。手元の M2 の max_random_00 では 13.14 ms で、lyndon_fs (7.19 ms) より遅かった。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_shared/linear_time_runs/runs.hpp"

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    const int n = int(s.size());
    string t(n + 2, '\0');
    copy(s.begin(), s.end(), t.begin() + 1);
    const auto runs = linear_time_runs::compute_all_runs(t.data(), t.size());
    vector<int> cnt(n / 2 + 2, 0);
    for (const auto &x : runs) ++cnt[x.period];
    for (int p = 0, acc = 0; p < int(cnt.size()); ++p) {
      const int k = cnt[p];
      cnt[p] = acc, acc += k;
    }
    ans.resize(runs.size());
    for (const auto &x : runs) ans[cnt[x.period]++] = {int(x.period), int(x.start) - 1, int(x.start) - 1 + int(x.length)};
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
