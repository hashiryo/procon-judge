// NeoLibrary の GeneralMatching (neo/algo/optimize/matching/GeneralMatching.hpp) で解く。帰着は common.hpp で、値の頂点と
// 0 の区間の頂点の一般グラフの最大マッチングを求める。頂点は 15 万ほどあるが、マッチングの大きさは値の数 (600) と偶数の区間の
// 数の和までにしかならない。
#include <vector>
#include "common.hpp"
#include "neo/algo/optimize/matching/GeneralMatching.hpp"
int main() {
 const cf1615g::Reduction R= cf1615g::read_and_build();
 const GeneralMatching gm(R.nv, R.edges);
 std::vector<int> mate(R.nv);
 for(int v= 0; v < R.nv; ++v) mate[v]= gm.mate(v);
 cf1615g::restore_and_print(R, mate);
}
