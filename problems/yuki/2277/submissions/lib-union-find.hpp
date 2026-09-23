#pragma once
#include "common.hpp"
#include "mylib/data_structure/UnionFind_Potentialized.hpp"

// 経路圧縮つきのポテンシャル付き Union-Find。潰しながら xor を畳む。
using Solver = ParitySolver<UnionFind_Potentialized<bool>>;
