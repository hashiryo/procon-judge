#pragma once
// マトロイドの和の書き比べ (マトロイドの題材の 5 回目)。台集合 [0, n) が同じ k 個のマトロイド M_0, ..., M_{k-1} から、
// 互いに素な独立集合 I_0, ..., I_{k-1} で、和集合の要素数が最大のものを求める。答えは要素ごとの番号 owner[x] (使わない
// 要素は -1)。
//
// union_bfs は Edmonds のマトロイドの分割の算法。要素を順に 1 つずつ入れ、どの M_j にもそのまま入らなければ、交換の列を
// 幅優先で探す。要素 u を I_j に入れると、I_j + u の閉路の要素 y が押し出され、y をほかの I_j' に入れ直す。どこかに
// そのまま入る要素に着いたら、列に沿って入れ替える。旗は次のとおり。
//   Frozen: 見つからなかった探索でたどった要素の集合 S は、S の要素をどの I_j に入れても S の要素しか押し出さず、そのまま
//           入る要素も無い。入れ替えで I_j と S の共通部分は変わらないのでこの性質はずっと保たれ、S は以後のどの探索の
//           最短の列にも現れない。そこで以後の探索では S の要素を飛ばす。
//   Fresh:  型の circuit_fresh を使う。fresh_reset から次の fresh_reset までに一度報告した要素を、二度報告しない閉路の
//           列挙 (グラフ的マトロイドは木の上で報告した辺を Union-Find で縮める)。
//   Add:    型の add を使う。I + x が独立なときに x を足し、can_add だけを正しく保つ (circuit は次の build まで使えない)。
//           そのまま入る要素ごとに build し直さずに済む。
//   Greedy: そのまま入る要素を先に全部入れてから、残りを探索で入れる。重みの順を守らなくてよいとき。
// union_isect は、k 個の直和と「同じ要素は 1 つまで」の分割マトロイドの交差に直して、NeoLibrary の matroid_intersection
// で解く。
#include <algorithm>
#include <array>
#include <vector>

#include "neo/algo/optimize/matroid/matroid_intersection.hpp"
#include "neo/algo/optimize/matroid/matroids.hpp"

namespace mu {
// グラフ的マトロイド。NeoLibrary の GraphicMatroid (e1f86af) に add、fresh_reset、circuit_fresh を足したもの。
class Graphic {
  int n_;
  std::vector<std::array<int, 2>> e_;
  std::vector<int> start_, adj_, comp_, dep_, pe_, pv_, tin_, tout_, it_, stk_;
  // add のあとの連結: 成分の根 (comp_ の値) の上の Union-Find。added_ のときだけ使う。
  std::vector<int> cu_;
  bool added_ = false;
  // circuit_fresh の Union-Find。fst_[v] == fnow_ の頂点 v は、親への辺を報告済みで、fu_[v] は v の祖先。
  std::vector<int> fu_;
  std::vector<unsigned> fst_;
  unsigned fnow_ = 1;
  int cfind(int v) {
    while (cu_[v] != v) v = cu_[v] = cu_[cu_[v]];
    return v;
  }
  int ffind(int v) {
    int r = v;
    while (fst_[r] == fnow_) r = fu_[r];
    while (fst_[v] == fnow_ && fu_[v] != r) {
      const int nx = fu_[v];
      fu_[v] = r, v = nx;
    }
    return r;
  }
public:
  Graphic(int n, const std::vector<std::array<int, 2>>& edges): n_(n), e_(edges), start_(n + 1), comp_(n), dep_(n), pe_(n), pv_(n), tin_(n), tout_(n), it_(n), cu_(n), fu_(n), fst_(n, 0) {}
  int size() const { return e_.size(); }
  void build(const std::vector<int>& I) {
    added_= false;
    std::fill(start_.begin(), start_.end(), 0);
    for(int y: I) ++start_[e_[y][0] + 1], ++start_[e_[y][1] + 1];
    for(int v= 0; v < n_; ++v) start_[v + 1]+= start_[v];
    adj_.resize(2 * I.size());
    for(int v= 0; v < n_; ++v) it_[v]= start_[v];
    for(int y: I) adj_[it_[e_[y][0]]++]= y, adj_[it_[e_[y][1]]++]= y;
    std::fill(comp_.begin(), comp_.end(), -1);
    for(int r= 0, t= 0; r < n_; ++r) {
      if(comp_[r] >= 0) continue;
      comp_[r]= r, dep_[r]= 0, pe_[r]= -1, pv_[r]= -1, tin_[r]= t++, it_[r]= start_[r];
      stk_.assign(1, r);
      while(!stk_.empty()) {
        const int v= stk_.back();
        if(it_[v] == start_[v + 1]) {
          tout_[v]= t, stk_.pop_back();
          continue;
        }
        const int y= adj_[it_[v]++], u= e_[y][0] ^ e_[y][1] ^ v;
        if(comp_[u] >= 0) continue;
        comp_[u]= r, dep_[u]= dep_[v] + 1, pe_[u]= y, pv_[u]= v, tin_[u]= t++, it_[u]= start_[u];
        stk_.push_back(u);
      }
    }
  }
  [[gnu::always_inline]] bool can_add(int x) {
    if(!added_) return comp_[e_[x][0]] != comp_[e_[x][1]];
    return cfind(comp_[e_[x][0]]) != cfind(comp_[e_[x][1]]);
  }
  void add(int x) {
    if(!added_) {
      added_= true;
      for(int v= 0; v < n_; ++v) cu_[v]= v;
    }
    cu_[cfind(comp_[e_[x][0]])]= cfind(comp_[e_[x][1]]);
  }
  [[gnu::always_inline]] bool can_exchange(int y, int x) {
    if(can_add(x)) return true;
    const int c= dep_[e_[y][0]] > dep_[e_[y][1]] ? e_[y][0] : e_[y][1];
    auto inside= [&](int v) { return tin_[c] <= tin_[v] && tin_[v] < tout_[c]; };
    return inside(e_[x][0]) != inside(e_[x][1]);
  }
  template <class F> void circuit(int x, F&& f) const {
    int u= e_[x][0], v= e_[x][1];
    while(u != v) {
      if(dep_[u] < dep_[v]) std::swap(u, v);
      f(pe_[u]), u= pv_[u];
    }
  }
  void fresh_reset() { ++fnow_; }
  template <class F> void circuit_fresh(int x, F&& f) {
    int a= ffind(e_[x][0]), b= ffind(e_[x][1]);
    while(a != b) {
      if(dep_[a] < dep_[b]) std::swap(a, b);
      f(pe_[a]);
      const int p= pv_[a];
      fst_[a]= fnow_, fu_[a]= p, a= ffind(p);
    }
  }
};

// GF(2) の線形マトロイド (64 bit のベクトル)。NeoLibrary の XorMatroid (e1f86af) に add、fresh_reset、circuit_fresh を
// 足したもの。x を基底で消した結果は覚えておき、基底が変わるまで使い回す。add は行を足すだけなので、消えた (従属だった)
// 要素の結果はそのまま使え、消えなかった (独立だった) 要素の結果だけが古くなる。そこで独立だった結果には add の回数の
// 印を別に付ける。build は、I が今の要素の並びと同じなら何もしない (union_bfs は add した順に I を持つので、add のあとの
// build はたいていこれになる)。
class Xor {
  std::vector<unsigned long long> v_, memo_;
  std::vector<int> elem_;
  std::vector<unsigned> stamp_, astamp_;
  unsigned long long row_[64]= {}, mask_[64]= {}, rep_= 0;
  unsigned now_= 1, adds_= 0;
  unsigned long long reduce(int x) {
    if(stamp_[x] == now_ && (memo_[x] != ~0ULL || astamp_[x] == adds_)) return memo_[x];
    stamp_[x]= now_;
    unsigned long long a= v_[x], m= 0;
    while(a) {
      const int b= 63 - __builtin_clzll(a);
      if(!row_[b]) return astamp_[x]= adds_, memo_[x]= ~0ULL;
      a^= row_[b], m^= mask_[b];
    }
    return memo_[x]= m;
  }
  void insert(int x) {
    const int k= elem_.size();
    elem_.push_back(x);
    unsigned long long a= v_[x], m= 1ULL << k;
    while(a) {
      const int b= 63 - __builtin_clzll(a);
      if(!row_[b]) {
        row_[b]= a, mask_[b]= m;
        break;
      }
      a^= row_[b], m^= mask_[b];
    }
  }
public:
  explicit Xor(const std::vector<unsigned long long>& v): v_(v), memo_(v.size()), stamp_(v.size(), 0), astamp_(v.size(), 0) {}
  int size() const { return v_.size(); }
  void build(const std::vector<int>& I) {
    if(I == elem_) return;
    elem_.clear();
    std::fill(row_, row_ + 64, 0);
    for(int y: I) insert(y);
    ++now_;
  }
  bool can_add(int x) { return reduce(x) == ~0ULL; }
  void add(int x) { insert(x), ++adds_; }
  template <class F> void circuit(int x, F&& f) {
    for(unsigned long long m= reduce(x); m; m&= m - 1) f(elem_[__builtin_ctzll(m)]);
  }
  void fresh_reset() { rep_= 0; }
  template <class F> void circuit_fresh(int x, F&& f) {
    const unsigned long long m= reduce(x) & ~rep_;
    rep_|= m;
    for(unsigned long long t= m; t; t&= t - 1) f(elem_[__builtin_ctzll(t)]);
  }
};

template <class M>
concept HasAdd= requires(M& m, int x) { m.add(x); };

// order を渡すと、その順に要素を入れる (Greedy は使えない)。渡さなければ 0, 1, ..., n - 1 の順。Add は型が add を持つ
// ときだけ効く。
template <bool Frozen, bool Fresh, bool Add, bool Greedy, class M> std::vector<int> union_bfs(std::vector<M>& ms, const std::vector<int>* order= nullptr) {
  const int k= ms.size(), n= k ? ms[0].size() : 0;
  std::vector<int> owner(n, -1), pos(n, -1), par(n), q;
  std::vector<std::vector<int>> I(k);
  // st[j]: 0 は I[j] で build 済み、1 は build のあと add だけした (can_add は正しい)、2 は作り直しが要る
  std::vector<char> st(k, 2), frozen(Frozen ? n : 0);
  std::vector<unsigned> vis(n, 0);
  unsigned stamp= 0;
  auto move= [&](int x, int j) {
    if(owner[x] >= 0) {
      auto& v= I[owner[x]];
      const int last= v.back();
      v[pos[x]]= last, pos[last]= pos[x], v.pop_back(), st[owner[x]]= 2;
    }
    owner[x]= j;
    if(j >= 0) pos[x]= I[j].size(), I[j].push_back(x), st[j]= 2;
  };
  auto direct= [&](int s) {
    for(int j= 0; j < k; ++j) {
      if(st[j] == 2) ms[j].build(I[j]), st[j]= 0;
      if(ms[j].can_add(s)) {
        if constexpr(Add && HasAdd<M>) {
          owner[s]= j, pos[s]= I[j].size(), I[j].push_back(s), ms[j].add(s);
          if(st[j] == 0) st[j]= 1;
        } else move(s, j);
        return true;
      }
    }
    return false;
  };
  auto search= [&](int s) {
    for(int j= 0; j < k; ++j)
      if(st[j] != 0) ms[j].build(I[j]), st[j]= 0;
    if constexpr(Fresh)
      for(auto& m: ms) m.fresh_reset();
    ++stamp;
    vis[s]= stamp, par[s]= -1;
    q.assign(1, s);
    int got= -1, got_j= -1;
    for(size_t h= 0; h < q.size() && got < 0; ++h) {
      const int u= q[h];
      auto visit= [&](int y) {
        if(got >= 0 || vis[y] == stamp) return;
        if constexpr(Frozen)
          if(frozen[y]) return;
        vis[y]= stamp, par[y]= u, q.push_back(y);
        for(int j= 0; j < k; ++j)
          if(j != owner[y] && ms[j].can_add(y)) {
            got= y, got_j= j;
            return;
          }
      };
      for(int j= 0; j < k && got < 0; ++j) {
        if(j == owner[u]) continue;
        if constexpr(Fresh) ms[j].circuit_fresh(u, visit);
        else ms[j].circuit(u, visit);
      }
    }
    if(got < 0) {
      if constexpr(Frozen)
        for(int y: q) frozen[y]= 1;
      return;
    }
    for(int x= got, j= got_j; x >= 0;) {
      const int old= owner[x], p= par[x];
      move(x, j), j= old, x= p;
    }
  };
  if(order) {
    for(int s: *order)
      if(!direct(s)) search(s);
  } else if constexpr(Greedy) {
    // 入れ替えでは I_j の張る空間が変わらず、そのまま入る要素で広がるだけなので、一度そのまま入らなかった要素は以後も
    // どの I_j にもそのまま入らない。2 周目は探索だけをする。
    std::vector<int> rest;
    for(int s= 0; s < n; ++s)
      if(!direct(s)) rest.push_back(s);
    for(int s: rest) search(s);
  } else {
    for(int s= 0; s < n; ++s)
      if(!direct(s)) search(s);
  }
  return owner;
}

// k 個の直和。要素 z は M_{z / n} の要素 z % n。
template <class M> class DirectSum {
  std::vector<M>& ms_;
  int n_, k_;
  std::vector<std::vector<int>> parts_;
public:
  explicit DirectSum(std::vector<M>& ms): ms_(ms), n_(ms.empty() ? 0 : ms[0].size()), k_(ms.size()), parts_(ms.size()) {}
  int size() const { return n_ * k_; }
  void build(const std::vector<int>& I) {
    for(auto& p: parts_) p.clear();
    for(int z: I) parts_[z / n_].push_back(z % n_);
    for(int j= 0; j < k_; ++j) ms_[j].build(parts_[j]);
  }
  bool can_add(int z) { return ms_[z / n_].can_add(z % n_); }
  bool can_exchange(int y, int x)
    requires matroid_internal::HasExchange<M>
  {
    const int jy= y / n_, jx= x / n_;
    return jy == jx ? ms_[jx].can_exchange(y % n_, x % n_) : ms_[jx].can_add(x % n_);
  }
  template <class F> void circuit(int x, F&& f) {
    const int j= x / n_;
    ms_[j].circuit(x % n_, [&](int y) { f(j * n_ + y); });
  }
};

template <class M> std::vector<int> union_isect(std::vector<M>& ms) {
  const int k= ms.size(), n= k ? ms[0].size() : 0;
  std::vector<int> owner(n, -1);
  if(k == 0) return owner;
  std::vector<int> part(k * n), cap(n, 1);
  for(int z= 0; z < k * n; ++z) part[z]= z % n;
  DirectSum<M> ds(ms);
  for(int z: matroid_intersection(ds, PartitionMatroid(part, cap))) owner[z % n]= z / n;
  return owner;
}
// NeoLibrary に置く候補の口。型は交差と同じく size、build、can_add と、can_exchange と circuit のうち書きやすいほうを持つ
// (circuit が無ければ、I の要素を全部 can_exchange で調べて閉路を作る)。add (I + x が独立なときに x を足し、can_add
// だけを正しく保つ) を持つ型は、そのまま入る要素ごとの build を省く。
// - matroid_union(ms): 和集合の要素数が最大になる割り当て (要素ごとの番号、使わない要素は -1)。
// - matroid_union(ms, order): order の順に要素を入れ、入らない要素は捨てる。和はマトロイドなので、重みの大きい順に
//   並べると、和集合の重みの和が最大になる (要素数も最大)。
template <class M> class UnionAdapter {
  M& m_;
  const std::vector<int>* I_= nullptr;
public:
  explicit UnionAdapter(M& m): m_(m) {}
  int size() const { return m_.size(); }
  void build(const std::vector<int>& I) { I_= &I, m_.build(I); }
  [[gnu::always_inline]] bool can_add(int x) { return m_.can_add(x); }
  void add(int x)
    requires HasAdd<M>
  {
    m_.add(x);
  }
  template <class F> void circuit(int x, F&& f) {
    if constexpr(matroid_internal::HasCircuit<M>) m_.circuit(x, f);
    else
      for(int y: *I_)
        if(m_.can_exchange(y, x)) f(y);
  }
};
template <class M> std::vector<int> matroid_union(std::vector<M>& ms) {
  std::vector<UnionAdapter<M>> as;
  as.reserve(ms.size());
  for(auto& m: ms) as.emplace_back(m);
  return union_bfs<true, false, true, true>(as);
}
template <class M> std::vector<int> matroid_union(std::vector<M>& ms, const std::vector<int>& order) {
  std::vector<UnionAdapter<M>> as;
  as.reserve(ms.size());
  for(auto& m: ms) as.emplace_back(m);
  return union_bfs<true, false, true, false>(as, &order);
}
}  // namespace mu
