#pragma once
// path-based (Gabow)。頂点ごとの記録に鍵と最初の隣接と残りの隣接の連結リストの先頭を並べる。剥がしは入れず、DFS だけで分ける。含意グラフは節の列から作り、yosupo-scc の同じ名前の提出と同じ核で分ける。包みは _two_sat.hpp、核は _scc_rec.hpp。
#include "_two_sat.hpp"
using Solver= two_sat::Solver<scc::GabowRecDFS<scc::RecList, true>>;
