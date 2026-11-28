#include <vector>
#include "_shared/dyntree/io.hpp"
#include "neo/base/ds/tree/OfflineDynamicForest.hpp"

struct SumG {
 using T = long long;
 using commute = void;
 static T ti() { return 0; }
 static T op(T a, T b) { return a + b; }
 static T inv(T a) { return -a; }
};

int main() {
 FastIn in;
 FastOut out;
 int n = in.read_int(), q = in.read_int();
 std::vector<long long> a(n);
 for (int i = 0; i < n; ++i) a[i] = (long long)in.read_u64();
 OfflineDynamicForest<SumG> df(n, a);
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32(), u = in.read_u32();
  if (t == 3) df.query(u);
  else if (t == 2) df.add(u, (long long)in.read_u32());
  else if (t == 0) df.link(u, in.read_u32());
  else df.cut(u, in.read_u32());
 }
 for (long long x : df.solve()) out.write_ll(x), out.nl();
}
