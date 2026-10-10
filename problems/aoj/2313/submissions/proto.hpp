#pragma once
// _shared/flow/proto_maxflow.hpp の試作の API で書いたもの。解き方は common.hpp と同じで、クエリで触る辺も容量 0 で先に
// 張っておき、change_cap で容量を 0 と 1 の間で変えて、上限 1 の flow で流し直す。辺は両向きに容量 1。
#include "_shared/flow/proto_maxflow.hpp"
struct Solver {
 int n;
 const vector<array<int, 2>>& edges;
 const vector<array<int, 3>>& qs;
 vector<i64> ans;
 Solver(int n, const vector<array<int, 2>>& edges, const vector<array<int, 3>>& qs): n(n), edges(edges), qs(qs) {}
 void run() {
  proto::MaxFlow<i64> g(n);
  vector<vector<int>> id(n, vector<int>(n, -1));
  for(auto& e: edges) id[e[0]][e[1]]= g.add_edge(e[0], e[1], 1, 1);
  for(auto& q: qs)
   if(id[q[1]][q[2]] == -1) id[q[1]][q[2]]= g.add_edge(q[1], q[2], 0, 0);
  i64 flow= g.flow(0, n - 1);
  ans.clear(), ans.reserve(qs.size());
  for(auto& q: qs) {
   const i64 c= q[0] == 1;
   flow-= g.change_cap(id[q[1]][q[2]], c, c, 0, n - 1);
   flow+= g.flow(0, n - 1, 1);
   ans.push_back(flow);
  }
 }
 const vector<i64>& answer() const { return ans; }
};
