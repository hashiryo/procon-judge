// NeoLibrary の gomory_hu (neo/algo/optimize/flow/gomory_hu.hpp) で Gomory-Hu 木を作り、common.hpp で木の最小の
// 辺で切りながら順列を組む。
#include <tuple>
#include <vector>
#include "common.hpp"
#include "neo/algo/optimize/flow/gomory_hu.hpp"
int main() {
 CF343E p;
 if(!p.read()) return 1;
 std::vector<std::tuple<int, int, int>> es;
 for(auto& [a, b, c]: p.e) es.push_back({a, b, c});
 auto g= gomory_hu(p.n, es);
 p.print(g.parent, std::vector<long long>(g.weight.begin(), g.weight.end()));
}
