#pragma once
// 診断用。gabow_rec_listb_trimpf2_fused_ds_hp を段に分けたもので、run() では、記録の初期化と、入次数も数えながら連結リストを組む走査だけを回す。段の分け方と仕組みは _diag_scc.hpp にある。
// 調べ終えたら消す。
#define DIAG_PHASE 1
#include "_diag_scc.hpp"
