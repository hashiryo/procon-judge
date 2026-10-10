#pragma once
// path-based (Gabow)。鍵、最初の隣接、残りの隣接の連結リストの先頭を別々の 4 byte の配列に置き、DFS の前に入次数 0 の頂点を剥がす。含意グラフは節の列から作り、yosupo-scc の同じ名前の提出と同じ核で分ける。包みは _two_sat.hpp、核は _scc_rec.hpp。
#include "_two_sat.hpp"
using Solver= two_sat::Solver<scc::GabowRecTrimPfFused<scc::RecSplit, true>>;
