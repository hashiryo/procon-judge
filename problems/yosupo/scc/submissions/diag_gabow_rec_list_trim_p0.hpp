#pragma once
// 診断用。gabow_rec_list_trim_hp を 4 つの段に分けたもので、作業領域を構築子で 0 で埋めてから、run() で 4 つの段をすべて回す。段の分け方と仕組みは _diag_scc.hpp にある。
// 調べ終えたら消す。
#define DIAG_PHASE 0
#include "_diag_scc.hpp"
