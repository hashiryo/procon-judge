// NeoLibrary の min_chain_cover (neo/combinatorial_optimization/path_cover.hpp) で解く。文書を、見られる開発者の集合の包含の
// 順序 (common.hpp) で最小の数の鎖に分け、鎖ごとに 1 つの組にする。鎖は前の文書の集合が後の文書の集合を含む向きで返るので、
// そのまま水準を付ける。関係は推移的なので min_path_cover でも同じ本数になるが、鎖を取り出す min_chain_cover を確かめる
// ためにこちらを使う。
#include <array>
#include <vector>
#include "common.hpp"
#include "neo/combinatorial_optimization/path_cover.hpp"
int main() {
 CF1765A p;
 if(!p.read()) return 1;
 std::vector<std::array<int, 2>> es;
 p.for_each_pair([&](int j, int k) { es.push_back({j, k}); });
 p.print(min_chain_cover(p.m, es));
}
