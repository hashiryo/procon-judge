#pragma once
#include "common.hpp"

// レベルグラフを作って行き止まりを刈りながら増加路を流す。素直な実装で、疎なグラフでは速い。
using Solver = FlowSolver<Dinic<i64>>;
