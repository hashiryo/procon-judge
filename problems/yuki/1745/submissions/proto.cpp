// _shared/flow/proto_bipartite.hpp の試作の API で書いたもの。辺 i を使う最大マッチングがあるのは edge_kind(i) != 0 のとき。
#include <array>
#include <cstdio>
#include <string>
#include <vector>
#include "_shared/flow/proto_bipartite.hpp"
int main() {
 int n, m, l;
 if(scanf("%d %d %d", &n, &m, &l) != 3) return 1;
 std::vector<std::array<int, 2>> es(l);
 for(auto& [a, b]: es) {
  if(scanf("%d %d", &a, &b) != 2) return 1;
  --a, --b;
 }
 proto::BipartiteMatching bm(n, m, es);
 std::string out;
 for(int i= 0; i < l; ++i) out+= bm.edge_kind(i) != 0 ? "Yes\n" : "No\n";
 std::fputs(out.c_str(), stdout);
}
