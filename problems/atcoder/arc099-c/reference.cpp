// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/arc099/tasks/arc099_c
// 2色塗り分け+連結成分
#include <iostream>
#include <algorithm>
#include <bitset>
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
 int N, M;
 cin >> N >> M;
 bool adj[N][N];
 for(int i= N; i--;)
  for(int j= i; j--;) adj[i][j]= adj[j][i]= 1;
 for(int i= M; i--;) {
  int A, B;
  cin >> A >> B, --A, --B;
  adj[A][B]= adj[B][A]= 0;
 }
 UnionFind_Potentialized<bool> uf(N);
 for(int i= N; i--;)
  for(int j= i; j--;)
   if(adj[i][j])
    if(!uf.unite(i, j, 1)) return cout << "-1\n", 0;
 int cnt[N];
 fill_n(cnt, N, 0);
 for(int i= N; i--;)
  if(uf.potential(i)) ++cnt[uf.leader(i)];
 bitset<701> dp;
 dp[0]= 1;
 for(int i= N; i--;) {
  if(uf.leader(i) != i) continue;
  dp= (dp << cnt[i]) | (dp << (uf.size(i) - cnt[i]));
 }
 int ans= N * N;
 for(int i= N + 1; i--;)
  if(dp[i]) ans= min(ans, i * (i - 1) / 2 + (N - i) * (N - i - 1) / 2);
 cout << ans << '\n';
 return 0;
}
