#include <array>
#include <vector>
#include "_shared/dyntree/io.hpp"
#include "_shared/dyntree/ett.hpp"

struct AddM {
 using T = long long;
 using E = long long;
 static T ti() { return 0; }
 static T op(T a, T b) { return a + b; }
 static void mp(T& v, E e, int len) { v += e * (long long)len; }
 static void cp(E& pre, E suf) { pre += suf; }
};

int main() {
 FastIn in;
 FastOut out;
 int n = in.read_int(), q = in.read_int();
 std::vector<long long> a(n);
 for (int i = 0; i < n; ++i) a[i] = (long long)in.read_u64();
 std::vector<std::array<int, 2>> edges(n - 1);
 for (int i = 0; i < n - 1; ++i) edges[i] = {in.read_int(), in.read_int()};
 EulerTourTree<AddM> ett(n, edges, a);
 for (int qi = 0; qi < q; ++qi) {
  int t = in.read_int();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   ett.cut(u, v), ett.link(w, x);
  } else if (t == 1) {
   int v = in.read_int(), p = in.read_int();
   long long x = (long long)in.read_u64();
   ett.apply_subtree(v, p, x);
  } else {
   int v = in.read_int(), p = in.read_int();
   out.write_ll(ett.prod_subtree(v, p)), out.nl();
  }
 }
}
