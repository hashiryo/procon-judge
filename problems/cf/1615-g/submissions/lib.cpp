// 今の Library の general_matching で解く。帰着は common.hpp で、値の頂点と 0 の区間の頂点の一般グラフの最大マッチングを求める。
#include <vector>
#include "common.hpp"
#include "mylib/graph/Graph.hpp"
#include "mylib/graph/general_matching.hpp"
int main() {
 const cf1615g::Reduction R= cf1615g::read_and_build();
 Graph g(R.nv);
 for(auto [u, v]: R.edges) g.add_edge(u, v);
 const auto [ids, mate]= general_matching(g);
 cf1615g::restore_and_print(R, mate);
}
