#include <utility>
#include <vector>
#include "_shared/dyntree/io.hpp"
#include "_shared/dyntree/lct_fast.hpp"

int main() {
  FastIn in;
  FastOut out;
  int n = in.read_int(), q = in.read_int();
  std::vector<long long> a(n);
  for (int i = 0; i < n; ++i) a[i] = (long long)in.read_u64();
  std::vector<std::pair<int, int>> edges(n - 1);
  for (auto &e : edges) e.first = in.read_int(), e.second = in.read_int();
  LctFast lct(a, edges);
  for (int qi = 0; qi < q; ++qi) {
    int t = in.read_int();
    if (t == 0) {
      int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
      lct.cut(u, v), lct.link(w, x);
    } else if (t == 1) {
      int v = in.read_int(), p = in.read_int();
      long long x = (long long)in.read_u64();
      lct.apply_subtree(v, p, x);
    } else {
      int v = in.read_int(), p = in.read_int();
      out.write_ll(lct.prod_subtree(v, p)), out.nl();
    }
  }
}
