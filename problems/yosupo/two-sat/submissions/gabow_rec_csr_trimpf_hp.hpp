#pragma once
// path-based (Gabow)。頂点ごとの記録に鍵と最初の隣接を並べ、残りの隣接は CSR。DFS の前に入次数 0 の頂点を先読みしながら剥がす。含意グラフは節の列から作り、yosupo-scc の同じ名前の提出と同じ核で分ける。包みは _two_sat.hpp、核は _scc_rec.hpp。
#include "_two_sat.hpp"
using Solver= two_sat::Solver<scc::GabowRecTrimPf<scc::RecCSR, true>>;
