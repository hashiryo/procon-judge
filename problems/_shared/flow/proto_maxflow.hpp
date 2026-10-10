#pragma once
// NeoLibrary に置く最大流の形の試作。ACL の mf_graph と同じく、add_edge で辺を足し (番号を返す)、flow(s, t) か
// flow(s, t, limit) で流し、min_cut(s) と get_edge(i) で解を取り出す。中身は hlpp_fifo と同じ highest label の
// push-relabel (同じ高さの中は FIFO、gap、global relabel)。flow のあとは常に正しい流れを持つ。前流の段で t へ流し、
// 届かなかった余りを、s を終点にしたもう一度の push-relabel で s へ戻す。流量の上限があるときは、s を普通の頂点と
// して上限の分だけ余りを持たせて始める (仮想の始点から容量 limit の辺で s に流し込むのと同じ)。辺は flow を呼んだ
// ときに CSR に組み、そのあと辺を足したら、流量を保ったまま組み直す。今の流れから続けて流すので、終点の違う
// flow を何度呼んでもよい。
#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <vector>
namespace proto {
template <class Cap> class MaxFlow {
 public:
  struct Edge {
   int from, to;
   Cap cap, flow;
  };
  explicit MaxFlow(int n= 0): n_(n) {}
  int add_vertex() { return sync(), built_= false, n_++; }
  // from から to へ容量 cap、逆向きに容量 rev_cap の辺。番号を返す。
  int add_edge(int from, int to, Cap cap, Cap rev_cap= 0) {
   assert(0 <= from && from < n_ && 0 <= to && to < n_ && cap >= 0 && rev_cap >= 0);
   sync();
   eu_.push_back(from), ev_.push_back(to), ecap_.push_back(cap), erev_.push_back(rev_cap), eflow_.push_back(0);
   built_= false;
   return int(eu_.size()) - 1;
  }
  // s から t へ流せるだけ流し、増えた量を返す。今の流れから続ける。
  Cap flow(int s, int t) { return run(s, t, false, 0); }
  // 増やす量を limit までにする。
  Cap flow(int s, int t, Cap limit) { return run(s, t, true, limit); }
  // 直前の flow のあとの残余グラフで s から届く頂点。
  std::vector<bool> min_cut(int s) {
   build();
   std::vector<bool> vis(n_);
   std::vector<int> q{s};
   vis[s]= true;
   for(size_t i= 0; i < q.size(); ++i)
    for(int k= st_[q[i]]; k < st_[q[i] + 1]; ++k)
     if(const A& a= as_[k]; a.cap > 0 && !vis[a.to]) vis[a.to]= true, q.push_back(a.to);
   return vis;
  }
  Edge get_edge(int i) {
   sync();
   return {eu_[i], ev_[i], ecap_[i], eflow_[i]};
  }
  int size() const { return n_; }
 private:
  struct A {
   int to, rev;
   Cap cap;
  };
  int n_;
  bool built_= false;
  std::vector<int> eu_, ev_;
  std::vector<Cap> ecap_, erev_, eflow_;
  std::vector<int> st_, pos_;  // pos_[i] = arc index of edge i (forward)
  std::vector<A> as_;
  std::vector<int> h_, cur_, anext_, atail_, ahead_, bnext_, bprev_, bhead_, q_;
  std::vector<Cap> ex_;
  void sync() {
   if(!built_) return;
   for(size_t i= 0; i < pos_.size(); ++i)
    if(pos_[i] >= 0) eflow_[i]= ecap_[i] - as_[pos_[i]].cap;
  }
  void build() {
   if(built_) return;
   const int n= n_, m= eu_.size();
   st_.assign(n + 1, 0), pos_.assign(m, -1);
   for(int i= 0; i < m; ++i)
    if(eu_[i] != ev_[i]) ++st_[eu_[i] + 1], ++st_[ev_[i] + 1];
   for(int v= 0; v < n; ++v) st_[v + 1]+= st_[v];
   as_.resize(st_[n]);
   std::vector<int> p(st_.begin(), st_.end() - 1);
   for(int i= 0; i < m; ++i)
    if(eu_[i] != ev_[i]) {
     const int a= p[eu_[i]]++, b= p[ev_[i]]++;
     as_[a]= {ev_[i], b, ecap_[i] - eflow_[i]}, as_[b]= {eu_[i], a, erev_[i] + eflow_[i]};
     pos_[i]= a;
    }
   h_.assign(n, 0), cur_.assign(n, 0), anext_.assign(n, -1), atail_.assign(n + 1, -1), ahead_.assign(n + 1, -1);
   bnext_.assign(n, -1), bprev_.assign(n, -1), bhead_.assign(n + 1, -1), q_.assign(n, 0), ex_.assign(n, 0);
   built_= true;
  }
  // 終点 sink に向けて、ex_ の余りを highest label で流す。excl は使わない頂点 (高さ n に置く)。
  void discharge_all(int sink, int excl) {
   const int n= n_;
   int amax= -1, bmax= -1;
   auto ins_b= [&](int v, int d) {
    bnext_[v]= bhead_[d], bprev_[v]= -1;
    if(bhead_[d] >= 0) bprev_[bhead_[d]]= v;
    bhead_[d]= v;
   };
   auto del_b= [&](int v, int d) {
    if(bprev_[v] >= 0) bnext_[bprev_[v]]= bnext_[v];
    else bhead_[d]= bnext_[v];
    if(bnext_[v] >= 0) bprev_[bnext_[v]]= bprev_[v];
   };
   auto ins_a= [&](int v, int d) {
    anext_[v]= -1;
    if(ahead_[d] < 0) ahead_[d]= atail_[d]= v;
    else anext_[atail_[d]]= v, atail_[d]= v;
    amax= std::max(amax, d);
   };
   auto global_relabel= [&] {
    std::fill(h_.begin(), h_.end(), n);
    std::fill(ahead_.begin(), ahead_.end(), -1), std::fill(bhead_.begin(), bhead_.end(), -1);
    amax= bmax= -1;
    int qh= 0, qt= 0;
    h_[sink]= 0, q_[qt++]= sink;
    while(qh < qt) {
     const int u= q_[qh++], d= h_[u] + 1;
     for(int k= st_[u]; k < st_[u + 1]; ++k)
      if(const A& a= as_[k]; h_[a.to] == n && a.to != excl && as_[a.rev].cap > 0) {
       h_[a.to]= d, q_[qt++]= a.to, cur_[a.to]= st_[a.to];
       ins_b(a.to, d), bmax= d;
       if(ex_[a.to] > 0) ins_a(a.to, d);
      }
    }
   };
   global_relabel();
   const long long freq= 2 * (6LL * n + st_[n] / 2);
   long long work= 0;
   while(amax >= 0) {
    const int v= ahead_[amax];
    if(v < 0) {
     --amax;
     continue;
    }
    ahead_[amax]= anext_[v];
    if(h_[v] != amax || ex_[v] == 0) continue;
    for(int hv= amax;;) {
     int k= cur_[v];
     for(const int ed= st_[v + 1]; k < ed; ++k)
      if(A& a= as_[k]; a.cap > 0 && h_[a.to] == hv - 1) {
       const int w= a.to;
       const Cap d= std::min(ex_[v], a.cap);
       if(w != sink && ex_[w] == 0) ins_a(w, hv - 1);
       a.cap-= d, as_[a.rev].cap+= d, ex_[v]-= d, ex_[w]+= d;
       if(ex_[v] == 0) break;
      }
     if(ex_[v] == 0) {
      cur_[v]= k;
      break;
     }
     int nh= n, nk= st_[v];
     for(int j= st_[v]; j < st_[v + 1]; ++j)
      if(const A& a= as_[j]; a.cap > 0 && h_[a.to] + 1 < nh) nh= h_[a.to] + 1, nk= j;
     work+= 12 + st_[v + 1] - st_[v];
     del_b(v, hv);
     if(bhead_[hv] < 0) {
      for(int d= hv; d <= bmax; ++d) {
       for(int u= bhead_[d]; u >= 0; u= bnext_[u]) h_[u]= n;
       bhead_[d]= -1;
      }
      bmax= hv - 1, h_[v]= n;
      break;
     }
     if(nh >= n) {
      h_[v]= n;
      break;
     }
     h_[v]= hv= nh, cur_[v]= nk;
     ins_b(v, nh), bmax= std::max(bmax, nh);
    }
    if(work > freq) work= 0, global_relabel();
   }
  }
  Cap run(int s, int t, bool limited, Cap limit) {
   assert(0 <= s && s < n_ && 0 <= t && t < n_ && s != t);
   build();
   std::fill(ex_.begin(), ex_.end(), Cap(0));
   if(limited) {
    // s を普通の頂点として、limit だけの余りを持たせる (仮想の始点から容量 limit の辺で s に流し込むのと同じ)。
    if(limit <= 0) return 0;
    ex_[s]= limit;
    discharge_all(t, -1);
    const Cap got= ex_[t];
    ex_[t]= 0, ex_[s]= 0;
    discharge_all(s, t);
    ex_[s]= 0;
    return got;
   }
   for(int k= st_[s]; k < st_[s + 1]; ++k)
    if(A& a= as_[k]; a.cap > 0) ex_[a.to]+= a.cap, as_[a.rev].cap+= a.cap, a.cap= 0;
   ex_[s]= 0;
   discharge_all(t, s);
   const Cap got= ex_[t];
   ex_[t]= 0;
   discharge_all(s, t);
   ex_[s]= 0;
   return got;
  }
};
}
