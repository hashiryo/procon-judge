#pragma once
// occ_gabow_trim_hp の剥がしで、待ち行列の 16 個先の頂点の出辺の範囲、8 個先の頂点の出辺の行き先、4 個先の頂点の最初の行き先の入次数を先読みする。核は _occ.hpp。
#include "_occ.hpp"
using Solver= occ::Solver<true>;
