#pragma once
// 診断用。gabow_rec_listb_trimpf2_fused_ds_hp を段に分けたもので、run() では、入次数 0 の頂点を剥がす段だけを回す。段の分け方と仕組みは _diag_scc.hpp にある。
// 調べ終えたら消す。
#define DIAG_PHASE 3
#include "_diag_scc.hpp"
