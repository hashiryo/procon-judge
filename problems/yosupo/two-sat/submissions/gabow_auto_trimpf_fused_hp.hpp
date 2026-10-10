#pragma once
// path-based (Gabow)。平均の出次数で隣接の持ち方を選ぶ (辺が頂点の 2 倍以上なら CSR、そうでなければ連結リスト)。含意グラフは節の列から作り、yosupo-scc の同じ名前の提出と同じ核で分ける。包みは _two_sat.hpp、核は _scc_rec.hpp。
#include "_two_sat.hpp"
using Solver= two_sat::Solver<scc::AutoAlg<scc::GabowRecTrimPfFused, true>>;
