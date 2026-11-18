#pragma once
// 増加路を 1 本ずつ探す二部マッチング (Kuhn 法) の書き比べの核。左 [0, L)、右 [0, R)。マッチングは空で始め、呼ぶ側が
// augment を呼んだときだけ増やす (左の頂点を足す順が答えを決める問題があるため)。
//
// 隣接は 1 本の配列 adj の上に、左の頂点ごとの区画 [beg, beg + len) として置く。構築で渡した辺は CSR と同じ並び (区画の
// 大きさは辺の数ちょうど) にし、add_edge で区画が溢れたら、2 倍の大きさの区画を配列の末尾に取って写す (区画の大きさの
// 表は初めて足すときに作る)。erase_edge(a, b) は a の区画から b を 1 つ探し、区画の末尾と入れ替えて抜く。同じ組を結ぶ
// 辺がほかに残っていれば、組はそのまま残す。
//
// 書き比べる手は 3 つで、テンプレートの引数で選ぶ。
// - KEEP: 0 なら、探すたびに訪ねた印を消す (教科書の形)。1 なら、増加路が見つからなかった探索で訪ねた左の頂点の印を、
//   次に増加路が見つかるまで残す。印の付いた左の頂点の集合 D からは、どの空いた右の頂点にも交互路で届かない (D の頂点の
//   隣の右の頂点はどれも埋まっていて、その相手も D にある)。次の探索は D へ入らない。2 なら、増加路が見つかっても D の
//   印を消さない (見つかった探索で訪ねた頂点の印だけを消す)。D の頂点の隣の右の頂点はどれも D の頂点と組んでいるので、
//   D の外を通る増加路はそこを通らず、流したあとも D の頂点の隣と相手は変わらない。どの形でも、印の付いた左の頂点から
//   辺を足したときは印を全部消す。2 では、組んだ辺を抜いたときも、その左の頂点に印が無ければ D は変わらない。
// - LA: 1 なら、左の頂点に入ったとき、まず隣に空いた右の頂点が無いかを全部見る (lookahead)。2 なら、見た位置を探索を
//   またいで覚え、続きから見る。右の頂点は一度埋まると、組んだ辺を抜くまで空かないので、覚えた位置より手前に空いた右の
//   頂点は無い (組んだ辺を抜いたら覚えた位置を全部捨て、区画の中で入れ替えたらその頂点の位置を戻す)。
// - BFS: 深さ優先でなく幅優先で探す (aoj-3198 の dyn と同じ)。
#include <algorithm>
#include <array>
#include <vector>
namespace kuhn {
template <int KEEP, int LA, bool BFS> class IncMatching {
public:
 IncMatching(int l, int r, const std::vector<std::array<int, 2>>& e= {}): L(l), R(r), beg(l + 1, 0), len(l, 0), adj(e.size()), ml(l, -1), mr(r, -1), vis(l, 0) {
  for(auto& x: e) ++len[x[0]];
  for(int i= 0; i < L; ++i) beg[i + 1]= beg[i] + len[i], len[i]= 0;
  for(auto& x: e) adj[beg[x[0]] + len[x[0]]++]= x[1];
  if constexpr(BFS) pre.assign(l, -1), q.reserve(l);
  else {
   const int d= std::min(l, r) + 1;
   stk.resize(d), via.resize(d), it.resize(d);
  }
  if constexpr(LA == 2) la.assign(l, 0), lep.assign(l, 0);
 }
 // 左 a と右 b を結ぶ辺を足す。マッチングは変えない。
 void add_edge(int a, int b) {
  if(cap.empty()) cap.assign(len.begin(), len.end());
  if(len[a] == cap[a]) {
   const int nb= adj.size(), nc= std::max(4, 2 * cap[a]);
   adj.resize(nb + nc);
   std::copy(adj.begin() + beg[a], adj.begin() + beg[a] + len[a], adj.begin() + nb);
   beg[a]= nb, cap[a]= nc;
  }
  adj[beg[a] + len[a]++]= b;
  if(vis[a] == stamp) ++stamp;  // a からは空いた右へ届かないと覚えていたが、辺が増えたので忘れる
 }
 // 左 a と右 b を結ぶ辺を 1 本抜く (無ければ何もしない)。組に使っていて、同じ組の辺がほかに無ければ、両端が空く。
 void erase_edge(int a, int b) {
  int* p= adj.data() + beg[a];
  const int j= std::find(p, p + len[a], b) - p;
  if(j == len[a]) return;
  p[j]= p[--len[a]];
  if constexpr(LA == 2) la[a]= std::min(la[a], j);
  if(ml[a] != b || std::find(p, p + len[a], b) != p + len[a]) return;
  if(KEEP != 2 || vis[a] == stamp) ++stamp;
  ml[a]= -1, mr[b]= -1, --sz, ++laep;
 }
 // 空いた左 a から増加路を探し、見つかれば流して true。a が空いていなければ false。
 bool augment(int a) {
  if(ml[a] >= 0) return false;
  if constexpr(KEEP == 0) ++stamp;
  return search(a);
 }
 // 空いた左の頂点のどれかから増加路を探し、見つかれば 1 本だけ流して true。
 bool augment() {
  if constexpr(KEEP == 0) ++stamp;
  if constexpr(BFS) {
   q.clear();
   for(int a= 0; a < L; ++a)
    if(ml[a] < 0 && vis[a] != stamp) vis[a]= stamp, pre[a]= -1, q.push_back(a);
   return bfs();
  } else {
   for(int a= 0; a < L; ++a)
    if(ml[a] < 0 && search(a)) return true;
   return false;
  }
 }
 int size() const { return sz; }
 int mate_left(int a) const { return ml[a]; }
 int mate_right(int b) const { return mr[b]; }
private:
 int L, R, sz= 0;
 unsigned stamp= 1, laep= 1;
 std::vector<int> beg, len, cap, adj;  // 左 l の隣は adj[beg[l], beg[l] + len[l])。cap は区画の大きさ (初めて足すときに作る)
 std::vector<int> ml, mr;               // 相手 (いなければ -1)
 std::vector<unsigned> vis, lep;
 std::vector<int> la, stk, via, it, pre, q, tv;  // tv は KEEP == 2 の深さ優先で訪ねた左の頂点
 bool search(int a) {
  if(vis[a] == stamp) return false;
  vis[a]= stamp;
  if constexpr(BFS) {
   pre[a]= -1, q.assign(1, a);
   return bfs();
  } else {
   if constexpr(KEEP == 2) tv.assign(1, a);
   int sp= 0;
   if(enter(a, sp)) return true;
   while(sp) {
    const int k= sp - 1, l= stk[k], n= len[l];
    const int* p= adj.data() + beg[l];
    int j= it[k], m= -1;
    for(; j < n; ++j) {
     if((m= mr[p[j]]) < 0) {
      via[k]= p[j], flip(sp);
      return true;
     }
     if(vis[m] != stamp) break;
    }
    if(j == n) {
     --sp;
     continue;
    }
    via[k]= p[j], it[k]= j + 1, vis[m]= stamp;
    if constexpr(KEEP == 2) tv.push_back(m);
    if(enter(m, sp)) return true;
   }
   return false;
  }
 }
 // l を積む。LA なら空いた右の隣を探し、見つかれば流して true。
 bool enter(int l, int& sp) {
  stk[sp]= l, it[sp]= 0, ++sp;
  if constexpr(LA != 0) {
   const int* p= adj.data() + beg[l];
   const int n= len[l];
   int j= 0;
   if constexpr(LA == 2) {
    if(lep[l] != laep) lep[l]= laep, la[l]= 0;
    j= la[l];
   }
   while(j < n && mr[p[j]] >= 0) ++j;
   if constexpr(LA == 2) la[l]= j;
   if(j < n) {
    via[sp - 1]= p[j], flip(sp);
    return true;
   }
  }
  return false;
 }
 // stk[0..sp) を via[0..sp) の右の頂点と組み直す。stk[k + 1] は via[k] の今の相手。
 void flip(int sp) {
  for(int k= 0; k < sp; ++k) ml[stk[k]]= via[k], mr[via[k]]= stk[k];
  ++sz;
  if constexpr(KEEP == 2)
   for(int l: tv) vis[l]= 0;
  else ++stamp;
 }
 // q に積んだ左の頂点 (印を付けてある) から幅優先で探す。pre[l] は l を見つけた左の頂点で、そのとき通った右の頂点は
 // 見つけたときの l の相手。
 bool bfs() {
  for(size_t h= 0; h < q.size(); ++h) {
   const int l= q[h], n= len[l];
   const int* p= adj.data() + beg[l];
   for(int j= 0; j < n; ++j) {
    const int r= p[j], m= mr[r];
    if(m < 0) {
     for(int x= l, y= r;;) {
      const int px= pre[x], old= ml[x];
      ml[x]= y, mr[y]= x;
      if(px < 0) break;
      x= px, y= old;
     }
     ++sz;
     if constexpr(KEEP == 2)
      for(int x: q) vis[x]= 0;
     else ++stamp;
     return true;
    }
    if(vis[m] != stamp) vis[m]= stamp, pre[m]= l, q.push_back(m);
   }
  }
  return false;
 }
};
}  // namespace kuhn
