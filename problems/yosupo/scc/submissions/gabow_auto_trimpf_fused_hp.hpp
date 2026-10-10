#pragma once
// path-based (Gabow)。平均の出次数で隣接の持ち方を選び (m >= 2 n なら記録と CSR、そうでなければ記録と連結リスト)、それぞれ gabow_rec_csr_trimpf_fused_hp と gabow_rec_list_trimpf_fused_hp と同じに回す。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _shared/scc/scc_rec.hpp。
#include "_shared/scc/scc_rec.hpp"
using Solver= scc::AutoSolver<scc::GabowRecTrimPfFused, true>;
