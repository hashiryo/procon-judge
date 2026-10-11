#pragma once
// Library の IncrementalBridgeConnectivity (mylib/graph/IncrementalBridgeConnectivity.hpp)。辺を 1 本ずつ足し、連結成分と
// 二辺連結成分を union-find で持つ。足した辺が同じ連結成分の 2 頂点を結べば、その間の木の道を 1 つの二辺連結成分に縮める。
// raw のときの lib.cpp と同じく、成分の名前は leader の頂点。
#include <optional>
#include "pj.hpp"
#include "mylib/graph/IncrementalBridgeConnectivity.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& es;
 mutable optional<IncrementalBridgeConnectivity> ibc;  // leader() が経路を縮めるので mutable
 Solver(int n, const vector<array<int, 2>>& es): n(n), es(es) {}
 void run() {
  ibc.emplace(n);
  for(auto& e: es) ibc->add_edge(e[0], e[1]);
 }
 int comp(int v) const { return ibc->leader(v); }
};
