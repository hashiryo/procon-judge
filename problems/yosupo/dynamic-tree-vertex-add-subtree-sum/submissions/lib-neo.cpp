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
 for (int i = 0; i < n - 1; ++i) df.link(in.read_int(), in.read_int());
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   df.cut(u, v), df.link(w, x);
  } else if (t == 1) {
   int p = in.read_int();
   df.add(p, (long long)in.read_u32());
  } else {
   int v = in.read_int(), p = in.read_int();
   df.cut(v, p), df.query(v), df.link(v, p);
  }
 }
 for (long long x : df.solve()) out.write_ll(x), out.nl();
}
