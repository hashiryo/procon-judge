#include <vector>
#include "_shared/dyntree/io.hpp"
#include "neo/base/ds/tree/OnlineDynamicConnectivity.hpp"

struct SumM {
 using T = long long;
 using commute = void;
 static T ti() { return 0; }
 static T op(T a, T b) { return a + b; }
};

int main() {
 FastIn in;
 FastOut out;
 int n = in.read_int(), q = in.read_int();
 std::vector<long long> a(n);
 for (int i = 0; i < n; ++i) a[i] = (long long)in.read_u64();
 OnlineDynamicConnectivity<SumM, 6'000'000> odc(n);
 for (int i = 0; i < n; ++i) odc.set(i, a[i]);
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32(), u = in.read_u32();
  if (t == 0) odc.link(u, in.read_u32());
  else if (t == 1) odc.cut(u, in.read_u32());
  else if (t == 2) odc.set(u, odc.get(u) + (long long)in.read_u32());
  else out.write_ll(odc.prod(u)), out.nl();
 }
}
