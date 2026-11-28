#include <vector>
#include "_shared/dyntree/io.hpp"
#include "_shared/dyntree/lct.hpp"

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
 LinkCutTree<SumM> lct(n);
 for (int i = 0; i < n; ++i) lct.init_value(i, a[i]);
 if (n > 1) {
  std::vector<int> head(n, -1), to(2 * (n - 1)), nxt(2 * (n - 1));
  int ec = 0;
  for (int i = 0; i < n - 1; ++i) {
   int u = in.read_int(), v = in.read_int();
   to[ec] = v, nxt[ec] = head[u], head[u] = ec++;
   to[ec] = u, nxt[ec] = head[v], head[v] = ec++;
  }
  std::vector<int> par(n, -1), st;
  st.reserve(n), st.push_back(0), par[0] = 0;
  while (!st.empty()) {
   int x = st.back();
   st.pop_back();
   for (int e = head[x]; e != -1; e = nxt[e])
    if (int y = to[e]; par[y] < 0) par[y] = x, st.push_back(y);
  }
  for (int v = 1; v < n; ++v) lct.init_parent(v, par[v]);
 }
 for (int qi = 0; qi < q; ++qi) {
  int t = in.read_int();
  if (t == 0) {
   int u = in.read_int(), v = in.read_int(), w = in.read_int(), x = in.read_int();
   lct.cut(u, v), lct.link(w, x);
  } else if (t == 1) {
   int p = in.read_int();
   lct.mul(p, (long long)in.read_u64());
  } else {
   int u = in.read_int(), v = in.read_int();
   out.write_ll(lct.prod(u, v)), out.nl();
  }
 }
}
