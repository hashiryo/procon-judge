#pragma once
// 一般グラフの最大マッチング。gm_dcover.hpp と同じく二部の二重被覆の最大マッチングから始めるが、二重被覆を辺の列に
// 組まず、もとのグラフの CSR を左右の両側の隣接としてそのまま使う push-relabel (pr_ks_half_pack と同じ手順) で解く。
//
// 左の u の隣は右の N(u)、右の v の隣は左の N(v) なので、どちらも adj[off[x]..off[x + 1]) になる。初期のマッチングは
// Karp-Sipser、そのあと double push を FIFO で回し、(2n) / 2 回組ませるごとに global relabel をかける。道と閉路に分けて
// 組ませ、gm_forest の交互森 (RELABEL) で仕上げるところは gm_dcover と同じ。
#include "_shared/flow/gm_forest.hpp"
namespace gm_dcpr {
// 二重被覆の最大マッチング。左 u の相手 (右の頂点、いなければ -1) を返す。init を渡すと、もとのグラフのマッチング init
// (相手の配列) の各辺を両向きに使った二重被覆のマッチングから始め、Karp-Sipser を飛ばす。
inline std::vector<int> dcover_matching(int n, const std::vector<int>& off, const std::vector<int>& adj, const std::vector<int>* init= nullptr) {
 const int INF= 2 * n;
 std::vector<int> ml(n, -1), mr(n, -1), q(n), bq(n);
 if(init) {
  for(int u= 0; u < n; ++u)
   if(const int v= (*init)[u]; v >= 0) ml[u]= v, mr[v]= u;
 } else {
  // Karp-Sipser。dl, dr は空いている隣の数。次数が 1 に落ちた頂点を stk に積む (右の頂点 r は n + r で積む)。
  std::vector<int> dl(n), dr(n), stk(2 * n);
  int sp= 0;
  for(int v= 0; v < n; ++v) {
   dl[v]= dr[v]= off[v + 1] - off[v];
   if(dl[v] == 1) stk[sp++]= v, stk[sp++]= n + v;
  }
  auto take= [&](int l, int r) {
   ml[l]= r, mr[r]= l;
   for(int k= off[l]; k < off[l + 1]; ++k)
    if(const int r2= adj[k]; mr[r2] < 0 && --dr[r2] == 1) stk[sp++]= n + r2;
   for(int k= off[r]; k < off[r + 1]; ++k)
    if(const int l2= adj[k]; ml[l2] < 0 && --dl[l2] == 1) stk[sp++]= l2;
  };
  for(int cur= 0;;) {
   while(sp) {
    const int x= stk[--sp];
    if(x < n) {
     if(ml[x] >= 0) continue;
     for(int k= off[x]; k < off[x + 1]; ++k)
      if(const int r= adj[k]; mr[r] < 0) {
       take(x, r);
       break;
      }
    } else {
     const int r= x - n;
     if(mr[r] >= 0) continue;
     for(int k= off[r]; k < off[r + 1]; ++k)
      if(const int l= adj[k]; ml[l] < 0) {
       take(l, r);
       break;
      }
    }
   }
   while(cur < n && (ml[cur] >= 0 || dl[cur] == 0)) ++cur;
   if(cur == n) break;
   for(int k= off[cur]; k < off[cur + 1]; ++k)
    if(const int r= adj[k]; mr[r] < 0) {
     take(cur, r);
     break;
    }
  }
 }
 struct Rv {
  int psi, mate;
 };
 std::vector<Rv> rv(n);
 for(int r= 0; r < n; ++r) rv[r].mate= mr[r];
 auto global_relabel= [&] {
  int h= 0, t= 0;
  for(int r= 0; r < n; ++r)
   if(rv[r].mate < 0) rv[r].psi= 0, bq[t++]= r;
   else rv[r].psi= INF;
  while(h < t) {
   const int r= bq[h++], d= rv[r].psi + 2;
   for(int k= off[r]; k < off[r + 1]; ++k)
    if(const int r2= ml[adj[k]]; r2 >= 0 && rv[r2].psi == INF) rv[r2].psi= d, bq[t++]= r2;
  }
 };
 global_relabel();
 int qh= 0, qn= 0;
 for(int l= 0; l < n; ++l)
  if(ml[l] < 0) q[qn++]= l;
 const int freq= std::max(1, n);
 for(int cnt= 0; qn;) {
  const int u= q[qh];
  if(++qh == n) qh= 0;
  --qn;
  int m1= INF, m2= INF, r1= -1;
  for(int k= off[u]; k < off[u + 1]; ++k) {
   const int r= adj[k], p= rv[r].psi;
   if(p < m1) m2= m1, m1= p, r1= r;
   else if(p < m2) m2= p;
  }
  if(m1 >= INF) continue;
  const int w= rv[r1].mate;
  ml[u]= r1, rv[r1]= {std::min(m2 + 2, INF), u};
  if(w >= 0) {
   ml[w]= -1;
   int t= qh + qn;
   if(t >= n) t-= n;
   q[t]= w, ++qn;
  }
  if(++cnt == freq) cnt= 0, global_relabel();
 }
 return ml;
}
// 二重被覆のマッチング (左 u の相手 out[u]) を道と閉路に分け、隣り合う 2 頂点ずつ組ませたマッチングを返す。
inline std::vector<int> round_dcover(int n, const std::vector<int>& out) {
 std::vector<int> in(n, -1), mate(n, -1);
 for(int u= 0; u < n; ++u)
  if(out[u] >= 0) in[out[u]]= u;
 std::vector<char> seen(n);
 auto walk= [&](int x) {
  while(x >= 0 && !seen[x]) {
   seen[x]= 1;
   const int y= out[x];
   if(y < 0 || seen[y]) break;
   seen[y]= 1, mate[x]= y, mate[y]= x, x= out[y];
  }
 };
 for(int v= 0; v < n; ++v)
  if(in[v] < 0) walk(v);
 for(int v= 0; v < n; ++v) walk(v);
 return mate;
}
inline std::vector<int> general_matching(int n, const std::vector<std::array<int, 2>>& e) {
 std::vector<int> off, adj;
 gm_gabow::build_csr(n, e, off, adj);
 std::vector<int> mate= round_dcover(n, dcover_matching(n, off, adj));
 gm_forest::finish<true>(n, off, adj, mate);
 return mate;
}
// Karp-Sipser のあと交互森を max_pass 回まで回し、終わらなければ、そのマッチングから二重被覆の push-relabel を始めて丸め、
// 交互森で仕上げる。増加路の短いグラフは森だけで終わり、長い増加路が残るグラフだけ二重被覆に回す。
inline std::vector<int> hybrid(int n, const std::vector<std::array<int, 2>>& e, int max_pass) {
 std::vector<int> off, adj;
 gm_gabow::build_csr(n, e, off, adj);
 std::vector<int> mate= gm_gabow::init_matching<2>(n, off, adj);
 if(gm_forest::finish<true>(n, off, adj, mate, max_pass)) return mate;
 mate= round_dcover(n, dcover_matching(n, off, adj, &mate));
 gm_forest::finish<true>(n, off, adj, mate);
 return mate;
}
}
