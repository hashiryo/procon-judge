#pragma once
// 診断用。snca_pk_hp の、順向きの隣接と DFS までを測り、残りは最初の parent() で回し直す。核は _dom.hpp。
#include "_dom.hpp"
using Solver= dom::Solver<3, true, 1>;
