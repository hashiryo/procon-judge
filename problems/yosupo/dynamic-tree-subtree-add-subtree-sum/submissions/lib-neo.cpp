#include <vector>
#include "_shared/dyntree/io.hpp"
#include "neo/base/ds/tree/AntiMonopolyTree.hpp"

int main() {
 FastIn in;
 FastOut out;
 int n = in.read_int(), q = in.read_int();
 std::vector<long long> a(n);
 for (int i = 0; i < n; ++i) a[i] = (long long)in.read_u64();
 OfflineSubtree<long long> dc(n);
 for (int i = 0; i < n - 1; ++i) dc.link(in.read_int(), in.read_int());
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   dc.cut(u, v), dc.link(w, x);
  } else if (t == 1) {
   int v = in.read_int(), p = in.read_int();
   long long x = (long long)in.read_u64();
   dc.cut(v, p), dc.add_all(v, x), dc.link(v, p);
  } else {
   int v = in.read_int(), p = in.read_int();
   dc.cut(v, p), dc.fold(v), dc.link(v, p);
  }
 }
 for (long long x : dc.solve(a)) out.write_ll(x), out.nl();
}
