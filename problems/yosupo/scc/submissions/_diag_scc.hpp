#pragma once
// 診断用の提出 diag_gabow_rec_listb_trimpf2_fused_ds_p0、p1、p3、p4 が共有する中身。gabow_rec_listb_trimpf2_fused_ds_hp
// (GabowRec<RecListB, true, 5>) の run() を段に分け、計測区間の run() では DIAG_PHASE の段だけを回す。それより前の段は構築子で、
// 後の段は count() で回すので、出力は正しいまま、その段だけの時間が記録に残る。どの段も、その段で初めて触るページの用意
// (0 埋め) を含む。
// 1: 記録の初期化と、辺の列から連結リストを組む走査 (入次数もここで数える)。
// 2: 入次数を数える走査。この版では 1 で数えるので空で、提出は置かない。
// 3: 入次数 0 の頂点を剥がす段。
// 4: 根の走査と、残りの DFS。
// DIAG_PHASE が 0 なら、作業領域を構築子ですべて 0 で埋めてから、run() で段をすべて回す (元の提出との差が、ページの用意の分)。
// 1 回目の最速の gabow_rec_list_trim_hp を同じように分けた診断の結果は、algo-notes の notes/graph-basics.md にある。
// 調べ終えたら消す。
#include "_scc_rec.hpp"
namespace diag_scc {
using A= scc::GabowRec<scc::RecListB, true, 5>;
struct Solver {
 const scc::Edges& es;
 mutable A a;
 mutable int done;  // 回し終えた段
 void step(int p) const {
  if(p == 1) a.build(es);
  else if(p == 2) a.count(es);
  else if(p == 3) a.peel();
  else a.dfs();
 }
 Solver(int n, const scc::Edges& es): es(es) {
  a.alloc(n, es.size());
  if constexpr(DIAG_PHASE == 0) memset(a.mem.base, 0, a.mem.bytes);
  for(int p= 1; p < DIAG_PHASE; ++p) step(p);
  done= DIAG_PHASE == 0 ? 0 : DIAG_PHASE - 1;
 }
 void run() {
  if constexpr(DIAG_PHASE == 0)
   for(int p= 1; p <= 4; ++p) step(p);
  else step(DIAG_PHASE);
  done= DIAG_PHASE == 0 ? 4 : DIAG_PHASE;
 }
 int count() const {
  while(done < 4) step(++done);
  return a.K;
 }
 int comp(int v) const { return a.comp(v); }
};
}
using Solver= diag_scc::Solver;
