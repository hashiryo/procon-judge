#include <vector>
#include "_shared/dyntree/io.hpp"
#include "neo/base/ds/tree/LinkCutTree.hpp"

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
 SubtreeLinkCutTree<SumG> lct(n);
 for (int i = 0; i < n; ++i) lct.set(i, a[i]);
 for (int i = 0; i < n - 1; ++i) {
  int u = in.read_int(), v = in.read_int();
  lct.link(u, v);
 }
 for (int qi = 0; qi < q; ++qi) {
  int t = in.read_int();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   lct.cut(u, v), lct.link(w, x);
  } else if (t == 1) {
   int p = in.read_int();
   lct.add(p, (long long)in.read_u64());
  } else {
   int v = in.read_int(), p = in.read_int();
   out.write_ll(lct.prod_subtree(v, p)), out.nl();
  }
 }
}
