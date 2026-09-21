#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきのポテンシャル付き Union-Find。潰しながら符号付き平行移動を畳む。
using Solver = CountSolver<UnionFind_Potentialized<G>>;
