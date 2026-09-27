// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// ポテンシャルUF
#include <iostream>
#include <vector>
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
 ios::sync_with_stdio(0);
 int N, M, Q;
 cin >> N >> M >> Q;
 UnionFind_Potentialized<long long> uf(N);
 vector<bool> isinf(N, false);
 for(int i= 0; i < M; i++) {
  int A, B;
  cin >> A >> B, A--, B--;
  long long C;
  cin >> C;
  if(!uf.unite(A, B, C)) isinf[A]= true, isinf[B]= true;
 }
 for(int i= 0; i < N; i++)
  if(isinf[i]) isinf[uf.leader(i)]= true;
 while(Q--) {
  int X, Y;
  cin >> X >> Y, X--, Y--;
  if(!uf.connected(X, Y)) cout << "nan" << '\n';
  else if(isinf[uf.leader(X)]) cout << "inf" << '\n';
  else cout << uf.diff(X, Y) << '\n';
 }
 return 0;
}
