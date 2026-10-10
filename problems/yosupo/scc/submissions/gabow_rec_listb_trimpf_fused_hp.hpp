#pragma once
// path-based (Gabow)。gabow_rec_list_trimpf_fused_hp の、連結リストを組む走査の「その頂点の最初の辺か」を、分岐にせずマスクで選ぶ。作業領域は 2 MB 境界の 1 本の領域で、huge page を頼む。核は _scc_rec.hpp。
#include "_scc_rec.hpp"
using Solver= scc::Solver<scc::GabowRecTrimPfFused, scc::RecListB, true>;
