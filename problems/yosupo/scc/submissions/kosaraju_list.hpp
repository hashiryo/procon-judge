#pragma once
// Kosaraju (順向きと逆向きの隣接を組み、2 回たどる)。隣接は辺の連結リスト。作業領域は配列ごとに malloc する。核は _scc.hpp。
#include "_scc.hpp"
using Solver= scc::Solver<scc::Kosaraju, scc::List, false>;
