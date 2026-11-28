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
 ops.reserve(std::size_t(2 * q + n));
 for (int i = 0; i < n - 1; ++i) {
  unsigned u = in.read_u32(), v = in.read_u32();
  ops.push_back({0u, u, v});
 }
 for (int i = 0; i < q; ++i) {
  unsigned t = in.read_u32();
  if (t == 0) {
   unsigned u = in.read_u32(), v = in.read_u32(), w = in.read_u32(), x = in.read_u32();
   ops.push_back({1u, u, v}), ops.push_back({0u, w, x});
  } else if (t == 1) {
   unsigned p = in.read_u32(), x = in.read_u32();
   ops.push_back({2u, p, x});
  } else {
   unsigned v = in.read_u32(), p = in.read_u32();
   ops.push_back({1u, v, p}), ops.push_back({3u, v, 0u}), ops.push_back({0u, v, p});
  }
 }
 auto ans = offline_dyn::solve<SumG>(a, ops);
 for (long long x : ans) out.write_ll(x), out.nl();
}
