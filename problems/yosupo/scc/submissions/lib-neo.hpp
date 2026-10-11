#pragma once
// NeoLibrary の strongly_connected_components (neo/graph/strongly_connected_components.hpp)。この問題で書き比べた
// gabow_split_trimpf_fused_hp を写したもので、作業領域が 2 MB 未満なら huge page を頼まない分と、成分の番号をトポロジカル順に
// 直した列を run() の中で作る分が違う。
#include "pj.hpp"
#include "neo/graph/strongly_connected_components.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 StronglyConnectedComponents r;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() { r= strongly_connected_components(n, es); }
 int count() const { return r.count; }
 int comp(int v) const { return r.comp[v]; }
};
