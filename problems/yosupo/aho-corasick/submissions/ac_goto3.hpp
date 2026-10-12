#pragma once
// ac_goto2 で、確保して 0 で埋める配列を減らし、残りに huge page を頼んだ版。ac_goto2 は文字の数の和 + 1 個分の par と link と、
// 深さで数え分ける表と順の配列と、頂点に入る文字の配列を 4 KB のページに取って 0 で埋めていた。CI (run 38209922611、EPYC 9V45)
// では頂点が 3 個しかない random_00 (1 文字の文字列が 10^6 本) でも 7.92 ms かかり、手元の M2 で段ごとに測ると、頂点が 10^6 個の
// 入力では確保だけで 1.3 ms から 1.7 ms、深さで数え分けるのに 1.1 ms から 2.5 ms かかった。ここでは (1) 幅優先の順を、深さで数え
// 分ける代わりに、取り出した頂点の子を行の欄から待ち行列に積んで作り (子の欄は suffix link の欄で埋める前に読む)、深さ、頂点に
// 入る文字、親から子への探索をなくす。(2) 遷移の表と待ち行列は huge page を頼んだ 1 つの領域に置き、答えの par、word、link は
// 領域を取って huge page を頼んでから 0 で埋める。中身は _ac_goto3.hpp。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_goto3.hpp"

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() { ac_goto3_body::solve(ss, ac_common::remap_total(ss), ans); }

  const AhoCorasickAnswer &answer() const { return ans; }
};
