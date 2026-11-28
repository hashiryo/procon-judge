#include <vector>
#include "_shared/dyntree/io.hpp"
#include "_shared/dyntree/lct.hpp"

constexpr unsigned MOD = 998244353;
struct CompM {
 using T = std::pair<unsigned, unsigned>;  // f(x) = a x + b
 static T ti() { return {1, 0}; }
 // f を先に、g をあとに当てる。
 static T op(const T& f, const T& g) {
  return {unsigned((unsigned long long)g.first * f.first % MOD), unsigned(((unsigned long long)g.first * f.second + g.second) % MOD)};
 }
};

int main() {
 FastIn in;
 FastOut out;
 int n = in.read_int(), q = in.read_int();
 LinkCutTree<CompM> lct(n);
 for (int i = 0; i < n; ++i) {
  unsigned a = in.read_u32(), b = in.read_u32();
  lct.init_value(i, {a, b});
 }
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
   unsigned c = in.read_u32(), d = in.read_u32();
   lct.set(p, {c, d});
  } else {
   int u = in.read_int(), v = in.read_int();
   unsigned x = in.read_u32();
   auto f = lct.prod(u, v);
   out.write_u64(((unsigned long long)f.first * x + f.second) % MOD);
   out.nl();
  }
 }
}
