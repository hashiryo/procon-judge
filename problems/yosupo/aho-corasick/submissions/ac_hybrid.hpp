#pragma once
// 子が 1 つの間は (文字, 子) を頂点に直接持ち、2 つ目の子ができた頂点にだけ、文字で直接引く幅 k の行を割り当てる版 (根は初めから行を
// 持つ)。行を持つのは枝分かれする頂点だけで、その数は文字列の本数以下になる。ac_goto2 は 26 文字で頂点が 10^6 個近くできる入力で
// 104 MB の遷移の行を全部埋める。子を兄弟のリストで持つ形 (手元だけで試した ac_sparse、置いていない) は子の多い頂点で兄弟をたどるのが
// 重かった (手元の random_11 で 166.73 ms、ac_goto2 は 48.85 ms) が、ほぼ 1 本道のトライ (short_period、almost_single、fibonacci)
// では ac_goto2 の 3 倍速かった。suffix link は 1 回目の ac_remap (0ae191d8、今は消した) と同じく深さの順に suffix link をたどって
// 求め、文字は使われているものだけに番号を振り直す (_ac_common.hpp)。
// 中身は _ac_hybrid.hpp。Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_hybrid.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() { ac_hybrid_body::solve(ss, ans); }

  const AhoCorasickAnswer &answer() const { return ans; }
};
