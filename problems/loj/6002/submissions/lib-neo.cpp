// NeoLibrary の min_path_cover (neo/algo/optimize/cover/path_cover.hpp) で解く。返す道をそのまま出す。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/algo/optimize/cover/path_cover.hpp"
int main() {
 int n, m;
 if(scanf("%d %d", &n, &m) != 2) return 1;
 std::vector<std::array<int, 2>> es(m);
 for(auto& [u, v]: es) {
  if(scanf("%d %d", &u, &v) != 2) return 1;
  --u, --v;
 }
 const auto paths= min_path_cover(n, es);
 std::string out;
 for(const auto& p: paths)
  for(size_t i= 0; i < p.size(); ++i) out+= std::to_string(p[i] + 1), out+= i + 1 < p.size() ? ' ' : '\n';
 out+= std::to_string(paths.size()), out+= '\n';
 fputs(out.c_str(), stdout);
}
