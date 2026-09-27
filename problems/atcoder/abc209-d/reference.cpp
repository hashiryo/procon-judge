// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// 偶奇
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
template <class weight_t> class UnionFind_Potentialized {
 std::vector<int> par;
 std::vector<weight_t> val;
public:
 UnionFind_Potentialized(int n): par(n, -1), val(n) {}
 int leader(int u) {
  if (par[u] < 0) return u;
  int r= leader(par[u]);
  if constexpr (std::is_same_v<weight_t, bool>) val[u]= val[u] ^ val[par[u]];
  else val[u]= val[par[u]] + val[u];
  return par[u]= r;
 }
 //  -p(v) + p(u) = w
 bool unite(int u, int v, weight_t w) {
  int a= leader(u), b= leader(v);
  if constexpr (std::is_same_v<weight_t, bool>) w^= val[u] ^ val[v];
  else w= val[v] + w - val[u];
  if (a == b) return w == weight_t();
  if (par[b] > par[a]) std::swap(a, b), w= -w;
  return par[b]+= par[a], par[a]= b, val[a]= w, true;
 }
 bool connected(int u, int v) { return leader(u) == leader(v); }
 int size(int u) { return -par[leader(u)]; }
 weight_t potential(int u) { return leader(u), val[u]; }
 //  -p(v) + p(u)
 weight_t diff(int u, int v) {
  if constexpr (std::is_same_v<weight_t, bool>) return potential(u) ^ potential(v);
  else return -potential(v) + potential(u);
 }
};
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int N, Q;
 cin >> N >> Q;
 UnionFind_Potentialized<bool> uf(N);
 for(int i= N - 1; i--;) {
  int a, b;
  cin >> a >> b;
  uf.unite(--a, --b, 1);
 }
 while(Q--) {
  int c, d;
  cin >> c >> d;
  cout << (uf.diff(--c, --d) ? "Road" : "Town") << '\n';
 }
 return 0;
}
