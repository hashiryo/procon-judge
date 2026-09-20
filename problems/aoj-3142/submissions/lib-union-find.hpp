#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきの重み付き Union-Find。潰しながら根までの重みを畳む。
using Solver = PotentialSolver<UnionFind_Potentialized<i64>>;
