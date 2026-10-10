#pragma once
// Library の StronglyConnectedComponents (mylib/graph/StronglyConnectedComponents.hpp)。Kosaraju で、Graph から順向きと
// 逆向きの CSR を組み、順向きの DFS の帰りがけ順の逆に、逆向きのグラフを BFS でたどって成分を集める。成分は見つけた順が
// そのままトポロジカル順になる。raw のときの lib.cpp と同じく、Graph に辺を写すところから測る。
#include <optional>
#include "pj.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/StronglyConnectedComponents.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 optional<StronglyConnectedComponents> scc;
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  Graph g(n, es.size());
  for(size_t i= 0; i < es.size(); ++i) g[i]= {es[i][0], es[i][1]};
  scc.emplace(g);
 }
 int count() const { return scc->size(); }
 int comp(int v) const { return (*scc)(v); }
};
