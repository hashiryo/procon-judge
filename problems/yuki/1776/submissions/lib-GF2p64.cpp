// https://yukicoder.me/problems/no/1776
// 友達でない 2 人を辺で結んだグラフで、X, Y, Z を通る最短の閉路を求める (Björklund-Husfeldt-Taslaman)。辺に GF2p64 の乱数の重みを置き、
// Y と Z を通る X から X への引き返さない歩道の重みの和を、長さを 1 ずつ伸ばして求める。標数 2 なので、最短の長さでは単純でない歩道の
// 項が対になって打ち消し合い、和が 0 でないことが、その長さの閉路があることになる (誤る確率はおよそ長さ / 2^64)。
// 辞書順で最小の閉路は、今の道の端から X へ戻る残りの最短の道を求め直しながら、次の頂点を 1 つずつ決めて作る
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "neo/algebra/GF2p64.hpp"
using namespace std;
mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
// s から t への道で need の頂点をすべて通るものの最小の長さと、s の次の頂点 (辞書順で最小のもの)。無ければ {-1, -1}。s = t でもよい
pair<int, int> find_path(const vector<vector<char>>& adj, int s, int t, const vector<int>& need) {
 const int N= adj.size(), K= need.size(), F= (1 << K) - 1;
 vector<int> fr, to, S, T;  // fr, to は向きのある辺で、2m と 2m+1 は互いに逆向き
 vector<GF2p64> wt;
 for(int b= 0; b < N; ++b)
  for(int a= 0; a < b; ++a) {
   if(!adj[a][b]) continue;
   if((a == s && b == t) || (a == t && b == s)) {
    if(!K) return {1, t};
    continue;
   }
   if(a == s || b == s) S.push_back(a ^ b ^ s);
   if(a == t || b == t) T.push_back(a ^ b ^ t);
   if(a != s && a != t && b != s && b != t) {
    const GF2p64 x(rng());
    fr.push_back(a), to.push_back(b), wt.push_back(x), fr.push_back(b), to.push_back(a), wt.push_back(x);
   }
  }
 vector<int> get(N);
 for(int k= 0; k < K; ++k) get[need[k]]= 1 << k;
 const int E= fr.size();
 // dp_v[mask][v] は v で終わる歩道、dp_e[mask][m] は辺 m で終わる歩道の重みの和 (mask は通った need)
 vector dp_v(F + 1, vector<GF2p64>(N)), nv= dp_v;
 vector dp_e(F + 1, vector<GF2p64>(E)), ne= dp_e;
 for(int v: T) dp_v[get[v]][v]= GF2p64(rng());
 for(int L= 1; L < N; ++L) {
  for(int v: S)
   if(dp_v[F][v]) return {1 + L, v};
  for(auto& r: nv) fill(r.begin(), r.end(), GF2p64());
  for(auto& r: ne) fill(r.begin(), r.end(), GF2p64());
  for(int m0= 0; m0 <= F; ++m0)
   for(int m= 0; m < E; ++m) {
    const int a= fr[m], b= to[m], m1= m0 | get[b];
    if(get[b] && m0 == m1) continue;
    const GF2p64 x= (dp_v[m0][a] + dp_e[m0][m ^ 1]) * wt[m];  // 直前に逆向きの辺を通った歩道を引く (引き返さない)
    ne[m1][m]+= x, nv[m1][b]+= x;
   }
  swap(dp_v, nv), swap(dp_e, ne);
 }
 return {-1, -1};
}
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N, M, X, Y, Z;
 cin >> N >> M >> X >> Y >> Z;
 --X, --Y, --Z;
 vector can(N, vector<char>(N, 1));
 for(int i= 0; i < M; ++i) {
  int a, b;
  cin >> a >> b;
  --a, --b;
  can[a][b]= can[b][a]= 0;
 }
 vector<int> path= {X};
 while(true) {
  vector<char> done(N);  // 道の途中の頂点 (両端を除く) はもう使えない
  for(size_t i= 1; i + 1 < path.size(); ++i) done[path[i]]= 1;
  vector adj(N, vector<char>(N));
  for(int a= 0; a < N; ++a)
   for(int b= 0; b < N; ++b) adj[a][b]= can[a][b] && !done[a] && !done[b];
  const int s= path.back();
  vector<int> need;
  for(int y: {Y, Z})
   if(y != s && y != X && !done[y]) need.push_back(y);
  const auto [L, nxt]= find_path(adj, s, X, need);
  if(L == -1) return cout << -1 << '\n', 0;
  path.push_back(nxt);
  if(nxt == X) break;
 }
 cout << path.size() - 1 << '\n';
 for(size_t i= 0; i < path.size(); ++i) cout << path[i] + 1 << " \n"[i + 1 == path.size()];
 return 0;
}
