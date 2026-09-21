#pragma once
#include "common.hpp"

// レベルグラフを作って行き止まりを刈りながら増加路を流す。
using Solver = TourSolver<Dinic<i64>>;
