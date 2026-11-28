#include <array>
#include <vector>
#include "_shared/dyntree/io.hpp"
#include "neo/base/ds/tree/EulerTourTree.hpp"

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
 for (int i = 0; i < n - 1; ++i) {
  int u = in.read_int(), v = in.read_int();
  edges[i] = {u, v};
 }
 EulerTourTree<AddM> ett(n, edges, a);
 for (int i = 0; i < q; ++i) {
  int t = in.read_int();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   ett.cut(u, v), ett.link(w, x);
  } else if (t == 1) {
   int v = in.read_int(), p = in.read_int();
   ett.apply_subtree(v, p, (long long)in.read_u64());
  } else {
   int v = in.read_int(), p = in.read_int();
   out.write_ll(ett.prod_subtree(v, p)), out.nl();
  }
 }
}
