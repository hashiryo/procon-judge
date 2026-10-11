#pragma once
// 最小費用の b-flow をネットワーク単体法で求める。木を LEMON の NetworkSimplex と同じ形で持つもの。
//
// 木の頂点を先行順に並べた thread (と逆向きの rev_thread) と、部分木の頂点の数 succ_num、部分木の thread の上で最後の頂点
// last_succ を持つ。部分木は thread の上で連続した区間になるので、pivot で動かした部分木のポテンシャルは、thread を
// last_succ までたどるだけで付け直せる (mcf_ns.hpp は子のリストを BFS でたどる)。最小の共通祖先は、succ_num の小さい
// ほうを親へ動かして求める。入る辺の選び方は block search (block の大きさは ceil(sqrt(m + n)) の BF_NUM / BF_DEN 倍)。
// 辺と頂点は配列ごとに分けて持つ。根 n と、各頂点から根へ (または根から) の人工の辺 (費用は費用の絶対値の和 + 1) から始める。
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
namespace mcf_ns2 {
using i64= long long;
struct Result {
 bool ok= false;
 __int128 cost= 0;
 std::vector<i64> pot, flow;
};
template <int BF_NUM= 1, int BF_DEN= 1> Result b_flow(int n, const std::vector<i64>& b, const std::vector<std::array<i64, 5>>& edges) {
 Result res;
 const int m= edges.size(), M= m + n, root= n;
 {
  i64 sum= 0;
  for(int v= 0; v < n; ++v) sum+= b[v];
  if(sum != 0) return res;
 }
 std::vector<int> src(M), dst(M);
 std::vector<i64> cap(M), flow(M, 0), cost(M);
 std::vector<int8_t> state(M, 1);  // 1 が下限、-1 が上限、0 が木
 std::vector<i64> sup(b.begin(), b.end());
 i64 art= 1;
 for(int e= 0; e < m; ++e) {
  const auto& [s, t, l, u, c]= edges[e];
  src[e]= (int)s, dst[e]= (int)t, cap[e]= u - l, cost[e]= c;
  sup[s]-= l, sup[t]+= l;
  art+= c < 0 ? -c : c;
 }
 // 木。pred_dir は木の辺が子から親へ向いていれば 1、親から子なら -1。
 std::vector<int> parent(n + 1), pred(n + 1), thread(n + 1), rev_thread(n + 1), succ_num(n + 1), last_succ(n + 1);
 std::vector<int8_t> pred_dir(n + 1);
 std::vector<i64> pi(n + 1);
 parent[root]= -1, pred[root]= -1, thread[root]= 0, rev_thread[0]= root, succ_num[root]= n + 1, last_succ[root]= root - 1, pi[root]= 0;
 for(int u= 0, e= m; u < n; ++u, ++e) {
  parent[u]= root, pred[u]= e, thread[u]= u + 1, rev_thread[u + 1]= u, succ_num[u]= 1, last_succ[u]= u;
  state[e]= 0, cost[e]= art;
  if(sup[u] >= 0) pred_dir[u]= 1, pi[u]= -art, src[e]= u, dst[e]= root, cap[e]= flow[e]= sup[u];
  else pred_dir[u]= -1, pi[u]= art, src[e]= root, dst[e]= u, cap[e]= flow[e]= -sup[u];
 }
 // root - 1 の次は根に戻る。
 thread[n - 1]= root, rev_thread[root]= n - 1;
 std::vector<int> dirty;
 const int block_size= std::max(1, std::max(int(std::ceil(std::sqrt((double)M))) * BF_NUM / BF_DEN, std::min(10, n + 1)));
 int next_arc= 0;
 for(;;) {
  // 入る辺 (block search)。
  int in_arc= -1;
  {
   i64 mn= 0;
   int cnt= block_size, e= next_arc;
   for(int seen= M; seen--; e= e + 1 == M ? 0 : e + 1) {
    if(const i64 c= state[e] * (cost[e] + pi[src[e]] - pi[dst[e]]); c < mn) mn= c, in_arc= e;
    if(--cnt == 0) {
     if(mn < 0) break;
     cnt= block_size;
    }
   }
   if(in_arc < 0) break;
   next_arc= e;
  }
  // 最小の共通祖先。
  int join;
  {
   int u= src[in_arc], v= dst[in_arc];
   while(u != v) {
    if(succ_num[u] < succ_num[v]) u= parent[u];
    else v= parent[v];
   }
   join= u;
  }
  // 出る辺。first から join と、second から join をたどる (後で見つけたものを優先して、木を strongly feasible に保つ)。
  int first, second, u_out= -1, result= 0;
  if(state[in_arc] == 1) first= src[in_arc], second= dst[in_arc];
  else first= dst[in_arc], second= src[in_arc];
  i64 delta= cap[in_arc];
  for(int u= first; u != join; u= parent[u]) {
   const int e= pred[u];
   const i64 d= pred_dir[u] == -1 ? cap[e] - flow[e] : flow[e];
   if(d < delta) delta= d, u_out= u, result= 1;
  }
  for(int u= second; u != join; u= parent[u]) {
   const int e= pred[u];
   const i64 d= pred_dir[u] == 1 ? cap[e] - flow[e] : flow[e];
   if(d <= delta) delta= d, u_out= u, result= 2;
  }
  int u_in, v_in;
  if(result == 1) u_in= first, v_in= second;
  else u_in= second, v_in= first;
  // 流す。
  if(delta > 0) {
   const i64 val= state[in_arc] * delta;
   flow[in_arc]+= val;
   for(int u= src[in_arc]; u != join; u= parent[u]) flow[pred[u]]-= pred_dir[u] * val;
   for(int u= dst[in_arc]; u != join; u= parent[u]) flow[pred[u]]+= pred_dir[u] * val;
  }
  if(result == 0) {
   state[in_arc]= -state[in_arc];
   continue;
  }
  state[in_arc]= 0;
  state[pred[u_out]]= flow[pred[u_out]] == 0 ? 1 : -1;
  // 木を付け替える (LEMON の updateTreeStructure)。u_out の部分木を切り離し、u_in から u_out までの幹の頂点の親を
  // 逆向きにして、u_in を v_in の子として付け直す。
  {
   const int old_rev_thread= rev_thread[u_out], old_succ_num= succ_num[u_out], old_last_succ= last_succ[u_out];
   const int v_out= parent[u_out];
   if(u_in == u_out) {
    parent[u_in]= v_in, pred[u_in]= in_arc, pred_dir[u_in]= u_in == src[in_arc] ? 1 : -1;
    if(thread[v_in] != u_out) {
     int after= thread[old_last_succ];
     thread[old_rev_thread]= after, rev_thread[after]= old_rev_thread;
     after= thread[v_in];
     thread[v_in]= u_out, rev_thread[u_out]= v_in;
     thread[old_last_succ]= after, rev_thread[after]= old_last_succ;
    }
   } else {
    const int thread_continue= old_rev_thread == v_in ? thread[old_last_succ] : thread[v_in];
    int stem= u_in, par_stem= v_in, last= last_succ[u_in], after= thread[last];
    thread[v_in]= u_in;
    dirty.clear(), dirty.push_back(v_in);
    while(stem != u_out) {
     const int next_stem= parent[stem];
     thread[last]= next_stem, dirty.push_back(last);
     const int before= rev_thread[stem];
     thread[before]= after, rev_thread[after]= before;
     parent[stem]= par_stem, par_stem= stem, stem= next_stem;
     last= last_succ[stem] == last_succ[par_stem] ? rev_thread[par_stem] : last_succ[stem];
     after= thread[last];
    }
    parent[u_out]= par_stem;
    thread[last]= thread_continue, rev_thread[thread_continue]= last;
    last_succ[u_out]= last;
    if(old_rev_thread != v_in) thread[old_rev_thread]= after, rev_thread[after]= old_rev_thread;
    for(int u: dirty) rev_thread[thread[u]]= u;
    int tmp_sc= 0;
    const int tmp_ls= last_succ[u_out];
    for(int u= u_out, p= parent[u]; u != u_in; u= p, p= parent[u]) {
     pred[u]= pred[p], pred_dir[u]= -pred_dir[p];
     tmp_sc+= succ_num[u] - succ_num[p];
     succ_num[u]= tmp_sc, last_succ[p]= tmp_ls;
    }
    pred[u_in]= in_arc, pred_dir[u_in]= u_in == src[in_arc] ? 1 : -1;
    succ_num[u_in]= old_succ_num;
   }
   const int up_limit_out= last_succ[join] == v_in ? join : -1;
   const int last_succ_out= last_succ[u_out];
   for(int u= v_in; u != -1 && last_succ[u] == v_in; u= parent[u]) last_succ[u]= last_succ_out;
   if(join != old_rev_thread && v_in != old_rev_thread) {
    for(int u= v_out; u != up_limit_out && last_succ[u] == old_last_succ; u= parent[u]) last_succ[u]= old_rev_thread;
   } else if(last_succ_out != old_last_succ) {
    for(int u= v_out; u != up_limit_out && last_succ[u] == old_last_succ; u= parent[u]) last_succ[u]= last_succ_out;
   }
   for(int u= v_in; u != join; u= parent[u]) succ_num[u]+= old_succ_num;
   for(int u= v_out; u != join; u= parent[u]) succ_num[u]-= old_succ_num;
  }
  // 動かした部分木のポテンシャルを付け直す。
  {
   const i64 sigma= pi[v_in] - pi[u_in] - pred_dir[u_in] * cost[in_arc];
   const int end= thread[last_succ[u_in]];
   for(int u= u_in; u != end; u= thread[u]) pi[u]+= sigma;
  }
 }
 for(int e= m; e < M; ++e)
  if(flow[e] != 0) return res;
 res.ok= true;
 res.pot.assign(pi.begin(), pi.begin() + n);
 res.flow.resize(m);
 for(int e= 0; e < m; ++e) res.flow[e]= flow[e] + edges[e][2], res.cost+= (__int128)res.flow[e] * edges[e][4];
 return res;
}
}
