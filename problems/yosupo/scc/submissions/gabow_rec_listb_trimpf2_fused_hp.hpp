#pragma once
// path-based (Gabow)。gabow_rec_listb_trimpf_fused_hp の剥がしを peel_pf2 にする (先読みの位置を待ち行列の末尾から切り離し、2 番目の隣接も番兵で分岐せずに扱う)。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc_rec.hpp。
#include "_scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecTrimPf2Fused, scc::RecListB, true>;
