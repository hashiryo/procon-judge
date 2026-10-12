#pragma once
// ac_hybrid3 に 2 つの変更を入れた版。(1) 行を持つ頂点 (根と枝分かれする頂点) の行の空いた欄を、取り出したときに遷移 delta(v, c) で
// 埋め (ac_goto3 と同じ埋め方を行のある頂点だけで行う)、suffix link をたどる途中で行のある頂点に着いたらその欄で止まる。(2) 頂点の
// 1 つ目の子とその文字、行の先頭、作業用の suffix link を 16 バイトの構造体にまとめ、行の無い頂点は 0 の行を指すようにして、子を
// 分岐なしで引く (1 つ目の子の文字が合えばその子、合わなければ行の欄)。3 回目 (7a0760d5 の run 38215267734、Xeon 8573C) の x64-gcc
// では、ac_hybrid3 は 1 本道のトライと 26 文字の入力で速い一方、2 文字から 5 文字のでたらめな文字列を多く並べた入力で ac_goto3 の
// 2 倍ほど遅かった (random_03 で 30.44 ms と 14.55 ms)。手元の M2 では、(1) だけの形 (ac_hybrid4、置いていない) で random_11 が
// 27.90 ms から 17.07 ms に縮み、(2) も入れると bfs_01 が 12.76 ms から 6.45 ms に縮んだ。random_03 は 24.47 ms から 21.53 ms で、
// ac_goto3 (8.70 ms) には届かない。suffix link は最後に答えの列へ写す。中身は _ac_hybrid5.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_hybrid5.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() { ac_hybrid5_body::solve(ss, ac_common::remap_total(ss), ans); }

  const AhoCorasickAnswer &answer() const { return ans; }
};
