#pragma once
// hlpp の global relabel の頻度を 8 分の 1 にしたもの。
// highest label の push-relabel (HIPR と同じ組み立て)。高さが最大の活性な頂点から余りを流し、流し切れなければ
// 高さを上げる。高さごとに、活性な頂点の片方向リストと、すべての頂点の両方向リストを持つ。ある高さの頂点が
// いなくなったら (gap)、それより上の頂点は t に届かないので高さを n にして外す。高さを上げた仕事量 (1 回につき
// 12 と見た辺の数) が 16(6n + m) を超えるたびに、t から逆向きの BFS で高さを正確な距離に直す (global relabel)。
// 求めるのは最大流の値だけなので、前流 (余りが t に届かない頂点に残った状態) を作った時点で止め、t の余りを返す。
// 辺は CSR に並べ、容量は int (入力が 2^31 - 1 以下なので残余容量も収まる)、余りは i64 で持つ。
#include <algorithm>
#include <array>
#include <vector>
namespace mf_hlpp_g16 {
using i64= long long;
struct E {
 int to, rev, cap;
};
inline i64 max_flow(int n, int s, int t, const std::vector<std::array<int, 3>>& edges) {
 std::vector<int> st(n + 1);
 for(auto& e: edges)
  if(e[0] != e[1]) ++st[e[0] + 1], ++st[e[1] + 1];
 for(int i= 0; i < n; ++i) st[i + 1]+= st[i];
 std::vector<E> es(st[n]);
 {
  std::vector<int> pos(st.begin(), st.end() - 1);
  for(auto& e: edges)
   if(e[0] != e[1]) {
    const int a= pos[e[0]]++, b= pos[e[1]]++;
    es[a]= {e[1], b, e[2]}, es[b]= {e[0], a, 0};
   }
 }
 std::vector<int> h(n), cur(n), anext(n), bnext(n), bprev(n), ahead(n + 1), bhead(n + 1), q(n);
 std::vector<i64> ex(n);
 int amax= -1, bmax= -1;
 auto ins_b= [&](int v, int d) {
  bnext[v]= bhead[d], bprev[v]= -1;
  if(bhead[d] >= 0) bprev[bhead[d]]= v;
  bhead[d]= v;
 };
 auto del_b= [&](int v, int d) {
  if(bprev[v] >= 0) bnext[bprev[v]]= bnext[v];
  else bhead[d]= bnext[v];
  if(bnext[v] >= 0) bprev[bnext[v]]= bprev[v];
 };
 auto ins_a= [&](int v, int d) { anext[v]= ahead[d], ahead[d]= v, amax= std::max(amax, d); };
 auto global_relabel= [&] {
  std::fill(h.begin(), h.end(), n);
  std::fill(ahead.begin(), ahead.end(), -1), std::fill(bhead.begin(), bhead.end(), -1);
  amax= bmax= -1;
  int qh= 0, qt= 0;
  h[t]= 0, q[qt++]= t;
  while(qh < qt) {
   const int u= q[qh++], d= h[u] + 1;
   for(int k= st[u]; k < st[u + 1]; ++k)
    if(const E& e= es[k]; h[e.to] == n && e.to != s && es[e.rev].cap > 0) {
     h[e.to]= d, q[qt++]= e.to;
     cur[e.to]= st[e.to];
     ins_b(e.to, d), bmax= d;
     if(ex[e.to] > 0) ins_a(e.to, d);
    }
  }
 };
 for(int k= st[s]; k < st[s + 1]; ++k)
  if(E& e= es[k]; e.cap > 0) ex[e.to]+= e.cap, es[e.rev].cap+= e.cap, e.cap= 0;
 global_relabel();
 const long long freq= 16 * (6LL * n + st[n] / 2);
 long long work= 0;
 while(amax >= 0) {
  const int v= ahead[amax];
  if(v < 0) {
   --amax;
   continue;
  }
  ahead[amax]= anext[v];
  if(h[v] != amax) continue;  // global relabel のあとに残った古い印
  // discharge
  for(int hv= amax;;) {
   int k= cur[v];
   for(const int ed= st[v + 1]; k < ed; ++k)
    if(E& e= es[k]; e.cap > 0 && h[e.to] == hv - 1) {
     const int w= e.to;
     const i64 d= std::min<i64>(ex[v], e.cap);
     if(w != t && ex[w] == 0) ins_a(w, hv - 1);
     e.cap-= d, es[e.rev].cap+= d, ex[v]-= d, ex[w]+= d;
     if(ex[v] == 0) break;
    }
   if(ex[v] == 0) {
    cur[v]= k;
    break;
   }
   // relabel
   int nh= n, nk= st[v];
   for(int j= st[v]; j < st[v + 1]; ++j)
    if(const E& e= es[j]; e.cap > 0 && h[e.to] + 1 < nh) nh= h[e.to] + 1, nk= j;
   work+= 12 + st[v + 1] - st[v];
   del_b(v, hv);
   if(bhead[hv] < 0) {
    // gap: hv より上の頂点は t に届かない
    for(int d= hv; d <= bmax; ++d) {
     for(int u= bhead[d]; u >= 0; u= bnext[u]) h[u]= n;
     bhead[d]= -1;
    }
    bmax= hv - 1;
    h[v]= n;
    break;
   }
   if(nh >= n) {
    h[v]= n;
    break;
   }
   h[v]= hv= nh, cur[v]= nk;
   ins_b(v, nh), bmax= std::max(bmax, nh);
  }
  if(work > freq) work= 0, global_relabel();
 }
 return ex[t];
}
}
struct Solver {
 int n, s, t;
 const vector<array<int, 3>>& e;
 long long ans= 0;
 Solver(int n, int s, int t, const vector<array<int, 3>>& edges): n(n), s(s), t(t), e(edges) {}
 void run() { ans= mf_hlpp_g16::max_flow(n, s, t, e); }
 long long answer() const { return ans; }
};
