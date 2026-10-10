#pragma once
// path-based (Gabow)。gabow_rec_listb_trimpf2_fused_hp の DFS の根の走査を、16 byte の記録でなく入次数の配列で行う。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _shared/scc/scc_rec.hpp。
#include "_shared/scc/scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecTrimPf2FusedDs, scc::RecListB, true>;
