#pragma once
// Kosaraju (順向きと逆向きの隣接を組み、2 回たどる)。隣接は CSR。作業領域は配列ごとに malloc する。核は _shared/scc/scc.hpp。
#include "_shared/scc/scc.hpp"
using Solver= scc::Solver<scc::Kosaraju, scc::CSR, false>;
