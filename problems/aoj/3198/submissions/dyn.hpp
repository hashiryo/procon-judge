#pragma once
// 最大マッチングを保ったまま、辺の追加と削除のたびに増加路を高々 1 本だけ探す。
//
// 辺を 1 本足しても抜いても、最大マッチングの大きさは高々 1 しか変わらない。使っていない辺を抜いたときは、今のマッチングが
// そのまま最大なので何もしない。使っている辺 (x, y) を抜いたときは、増加路があれば x から出るか y に入るので、空いている
// 左の頂点すべてから交互路を BFS で探す (完全マッチングだったなら、空いている左の頂点は x だけ)。辺を足したときは、
// 足す前のマッチングが最大なので、増加路があれば足した辺を通る。これも空いている左の頂点すべてから探す。隣接は左の頂点
// ごとの配列に持ち、抜くときは末尾と入れ替える。初めのマッチングは、空いている左の頂点から 1 本ずつ増加路を探して作る。
#include <algorithm>
#include <vector>
struct Solver {
 int n, sz= 0, stamp= 0;
 std::vector<std::vector<int>> adj;  // 左 → 右
 std::vector<int> ml, mr, vis, par, q;
 Solver(int n, const vector<array<int, 2>>& edges): n(n), adj(n), ml(n, -1), mr(n, -1), vis(n, 0), par(n) {
  for(auto& e: edges) adj[e[0]].push_back(e[1]);
  q.reserve(n);
  for(int s= 0; s < n; ++s)
   if(ml[s] < 0) sz+= augment_from({s});
 }
 // srcs (空いている左の頂点) から交互路を BFS で探し、空いている右の頂点に着いたら流して true を返す。vis と par は
 // 右の頂点に付ける (par[r] は r を見つけた左の頂点)。
 bool augment_from(const std::vector<int>& srcs) {
  ++stamp;
  q.assign(srcs.begin(), srcs.end());
  for(size_t h= 0; h < q.size(); ++h) {
   const int l= q[h];
   for(int r: adj[l]) {
    if(vis[r] == stamp) continue;
    vis[r]= stamp, par[r]= l;
    if(mr[r] < 0) {
     // r から逆にたどって流す。
     for(int x= r; x >= 0;) {
      const int y= par[x], nx= ml[y];
      ml[y]= x, mr[x]= y, x= nx;
     }
     return true;
    }
    q.push_back(mr[r]);
   }
  }
  return false;
 }
 bool toggle(int x, int y) {
  auto& a= adj[x];
  if(auto it= std::find(a.begin(), a.end(), y); it != a.end()) {
   *it= a.back(), a.pop_back();
   if(ml[x] == y) ml[x]= mr[y]= -1, --sz, sz+= augment_from(free_left());
  } else {
   a.push_back(y);
   if(sz < n) sz+= augment_from(free_left());
  }
  return sz == n;
 }
 std::vector<int> free_left() const {
  std::vector<int> srcs;
  for(int l= 0; l < n; ++l)
   if(ml[l] < 0) srcs.push_back(l);
  return srcs;
 }
};
