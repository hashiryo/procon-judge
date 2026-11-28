#include <vector>
#include "_shared/dyntree/io.hpp"
#include "_shared/dyntree/offline_forest.hpp"

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
 std::vector<offline_dyn::Operation> ops;
 ops.reserve(q);
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32();
  unsigned u = in.read_u32();
  unsigned v = t == 3 ? 0u : in.read_u32();
  ops.push_back({t, u, v});
 }
 auto ans = offline_dyn::solve<SumG>(a, ops);
 for (long long x : ans) out.write_ll(x), out.nl();
}
