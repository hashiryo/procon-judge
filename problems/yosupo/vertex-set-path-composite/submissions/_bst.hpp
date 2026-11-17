#pragma once
#include "_hld.hpp"
// Library Checker の首位 (chaihf の 270199) と同じ、HLD の鎖ごとに二分木を組む形。1 回の問い合わせと更新が O(log N)。
// 位置は 1 始まりの行きがけ順で、軽い子を先に、重い子を最後に配る。そうすると鎖の隣どうしの位置の間に、軽い子の
// 部分木の大きさだけ隙間が空く。鎖の頂点の優先度を「位置と鎖の次の頂点の位置の xor の最上位 bit」とし (鎖の最後は
// 次の位置)、鎖ごとに優先度の大きい方を根にした二分木 (位置の順の Cartesian 木) を組む。隙間の大きい所ほど根に近く
// なるので、木の全体で、ある頂点から鎖の二分木の根までの深さの和が O(log N) に収まる。
// 節は自分の 1 次関数と、部分木の積 s と、左の部分木と自分の積 t を持つ。積は (掛ける数、上りの足す数、下りの足す数)
// で、1 次関数は合成の向きで掛ける数が変わらないので 3 つで両向きを持てる。値は Montgomery (_mont.hpp)。
// 鎖の頂点 k から鎖の先頭までの積は、k から二分木の根へ上がり、位置が k 以下の祖先の t を集めれば得られる。
namespace comp_bst {
struct S3 {
 u32 a, b, c;  // x -> a x + b が上り (位置の大きい方を先に当てる)、x -> a x + c が下り
};
// l が位置の小さい側 (根に近い側)。
inline S3 cat(const S3& l, const S3& r) { return {mont::mul(l.a, r.a), mont::mulsum(l.a, r.b, l.b, mont::R1), mont::mulsum(r.a, l.c, r.c, mont::R1)}; }
inline u32 up(const S3& f, u32 x) { return mont::mulsum(f.a, x, f.b, mont::R1); }
inline u32 down(u32 x, const S3& f) { return mont::mulsum(f.a, x, f.c, mont::R1); }
struct Node {
 u32 va, vb;
 S3 s, t;
 int l, r, p;
};
struct Tree {
 int n;
 comp_hld::Buf buf;
 int* pos;     // 頂点 → 位置 (1 始まり)
 u64* ch;      // 位置 → 鎖の先頭の位置と、先頭の親の位置 (根の鎖は 0) の組
 Node* nd;     // 位置 → 節。nd[0] は番兵で、s は恒等
 unsigned char* pr;  // 位置 → 優先度
 Tree(int n, const vector<array<i64, 2>>& f, const vector<array<int, 2>>& edges, int root= 0): n(n) {
  // 確保: ch (n + 1 個の u64)、nd (n + 1 個)、pos (n 個)、pr (n + 1 byte)
  const size_t words= 2 * size_t(n + 1) + sizeof(Node) / 4 * size_t(n + 1) + size_t(n) + size_t(n + 1) / 4 + 1;
  buf= comp_hld::alloc_huge(words);
  ch= (u64*)buf.get(), nd= (Node*)(ch + n + 1), pos= (int*)(nd + n + 1), pr= (unsigned char*)(pos + n);
  comp_hld::Buf tmp= comp_hld::alloc_huge(6 * size_t(n) + 2);
  int *ord= tmp.get(), *par= ord + n, *sz= par + n, *hb= sz + n + 1;
  int* dx= hb;  // dx[2v] = 次数、dx[2v + 1] = 隣の番号の xor
  std::fill(dx, dx + 2 * size_t(n), 0);
  for(const auto& e: edges) {
   const int a= e[0], b= e[1];
   ++dx[2 * a], dx[2 * a + 1]^= b, ++dx[2 * b], dx[2 * b + 1]^= a;
  }
  dx[2 * root]+= 2;
  int k= 0;
  for(int v= 0; v < n; ++v)
   for(int u= v; dx[2 * u] == 1;) {
    const int p= dx[2 * u + 1];
    ord[k++]= u, par[u]= p, dx[2 * u]= 0;
    dx[2 * p + 1]^= u, --dx[2 * p];
    u= p;
   }
  ord[k]= root, par[root]= -1;
  std::fill(sz, sz + n, 1), std::fill(hb, hb + 2 * size_t(n), 0);
  for(int i= 0; i + 1 < n; ++i) {
   const int v= ord[i], p= par[v], s= sz[v];
   sz[p]+= s;
   if(s > hb[2 * p]) hb[2 * p]= s, hb[2 * p + 1]= v;
  }
  // 上から下へ位置を配る。軽い子は親の次から順に、重い子は親の部分木の最後に置く。nxt[v] は v の次の軽い子の位置。
  // par[v] は v の位置を配るときに読み終えるので、その欄を nxt に使い回す。
  int* nxt= par;
  nd[0]= Node{0, 0, S3{mont::R1, 0, 0}, S3{mont::R1, 0, 0}, 0, 0, 0};
  pr[0]= 0;
  auto place= [&](int v, int x, int up_pos, bool heavy, int prev) {
   pos[v]= x;
   const int s= sz[v];
   pr[x]= (unsigned char)(31 - __builtin_clz(unsigned(x ^ (x + s - hb[2 * v]))));
   const u32 a= mont::enc(f[v][0]), b= mont::enc(f[v][1]);
   nd[x].va= a, nd[x].vb= b, nd[x].l= nd[x].r= 0;
   if(heavy) {
    ch[x]= ch[prev];
    int q= prev, l= 0;
    while(q && pr[q] < pr[x]) l= q, q= nd[q].p;
    nd[q].r= x, nd[x].p= q, nd[x].l= l, nd[l].p= x;
   } else {
    ch[x]= comp_hld::chain(x, up_pos), nd[x].p= 0;
   }
  };
  place(root, 1, 0, false, 0);
  nxt[root]= 2;
  for(int i= n - 1; i--;) {
   const int v= ord[i], p= par[v], pp= pos[p];
   const bool heavy= hb[2 * p + 1] == v;
   int x;
   if(heavy) x= pp + sz[p] - sz[v];
   else x= nxt[p], nxt[p]+= sz[v];
   place(v, x, pp, heavy, pp);
   nxt[v]= x + 1;
  }
  nd[0].s= S3{mont::R1, 0, 0}, nd[0].p= nd[0].l= nd[0].r= 0;
  // 二分木の帰りがけ順で s と t を作る。右の子の無い節 i から、位置が i 以下の祖先へ上がりながら作ると、
  // どの節も左右の部分木を作り終えてから作られる (i はそれらの祖先の部分木の位置の最大)。
  for(int i= 1; i <= n; ++i)
   if(nd[i].r == 0)
    for(int u= i; u && u <= i; u= nd[u].p) maintain(u);
 }
 void maintain(int u) {
  Node& x= nd[u];
  x.t= cat(nd[x.l].s, S3{x.va, x.vb, x.vb});
  x.s= cat(x.t, nd[x.r].s);
 }
 void set(int v, i64 c, i64 d) {
  int u= pos[v];
  nd[u].va= mont::enc(c), nd[u].vb= mont::enc(d);
  for(; u; u= nd[u].p) maintain(u);
 }
 // 鎖の頂点 k から鎖の先頭へ、上りの向きで x に当てる。
 u32 up_prefix(int k, u32 x) const {
  for(int u= k; u; u= nd[u].p)
   if(u <= k) x= up(nd[u].t, x);
  return x;
 }
 // 鎖の先頭から k までの積を f の前に付ける。
 S3 down_prefix(int k, S3 f) const {
  for(int u= k; u; u= nd[u].p)
   if(u <= k) f= cat(nd[u].t, f);
  return f;
 }
 // 同じ鎖の位置 [l, r] の積を、r から l へ上りで x に当てる。
 u32 up_range(int l, int r, u32 x) const {
  S3 t{mont::R1, 0, 0};
  int u= l, v= r;
  while(u != v) {
   if(pr[u] < pr[v]) {
    if(l <= u) t= cat(cat(t, S3{nd[u].va, nd[u].vb, nd[u].vb}), nd[nd[u].r].s);
    u= nd[u].p;
   } else {
    if(v <= r) x= up(nd[v].t, x);
    v= nd[v].p;
   }
  }
  return up(t, mont::mulsum(nd[u].va, x, nd[u].vb, mont::R1));
 }
 // 同じ鎖の位置 [l, r] の積を f の前に付ける。
 S3 down_range(int l, int r, S3 f) const {
  S3 t{mont::R1, 0, 0};
  int u= l, v= r;
  while(u != v) {
   if(pr[u] < pr[v]) {
    if(l <= u) t= cat(cat(t, S3{nd[u].va, nd[u].vb, nd[u].vb}), nd[nd[u].r].s);
    u= nd[u].p;
   } else {
    if(v <= r) f= cat(nd[v].t, f);
    v= nd[v].p;
   }
  }
  return cat(t, cat(S3{nd[u].va, nd[u].vb, nd[u].vb}, f));
 }
 // composite と同じ答えを、1 次関数どうしを合成せずに値へ 1 回ずつ当てて求める。u の側は見つけた順に当て、v の側の
 // 鎖の接頭辞と、最後の同じ鎖の区間の右側の節は、番号を積んでおいて最後に逆順で下りの向きに当てる。最後の区間が
 // 上りなら、左側の節を積んでおき、真ん中の節を当てたあとに逆順で当てる。
 u32 applyv(int u, u32 x) const { return mont::mulsum(nd[u].va, x, nd[u].vb, mont::R1); }
 i64 composite_apply(int a0, int b0, i64 x0) const {
  int x= pos[a0], y= pos[b0];
  u64 a= ch[x], b= ch[y];
  u32 val= mont::enc(x0);
  int st[256], k= 0;
  while(comp_hld::head_of(a) != comp_hld::head_of(b)) {
   if(comp_hld::head_of(a) > comp_hld::head_of(b)) {
    for(int u= x; u; u= nd[u].p)
     if(u <= x) val= up(nd[u].t, val);
    x= comp_hld::up_of(a), a= ch[x];
   } else {
    for(int u= y; u; u= nd[u].p)
     if(u <= y) st[k++]= u;
    y= comp_hld::up_of(b), b= ch[y];
   }
  }
  if(x >= y) {
   const int l= y, r= x;
   int u= l, v= r, ls[64], nl= 0;
   while(u != v) {
    if(pr[u] < pr[v]) {
     if(l <= u) ls[nl++]= u;
     u= nd[u].p;
    } else {
     if(v <= r) val= up(nd[v].t, val);
     v= nd[v].p;
    }
   }
   val= applyv(u, val);
   while(nl--) val= applyv(ls[nl], up(nd[nd[ls[nl]].r].s, val));
  } else {
   const int l= x, r= y;
   int u= l, v= r;
   while(u != v) {
    if(pr[u] < pr[v]) {
     if(l <= u) val= down(applyv(u, val), nd[nd[u].r].s);
     u= nd[u].p;
    } else {
     if(v <= r) st[k++]= v;
     v= nd[v].p;
    }
   }
   val= applyv(u, val);
  }
  while(k) val= down(val, nd[st[--k]].t);
  return mont::dec(val);
 }
 i64 composite(int a0, int b0, i64 x0) const {
  int x= pos[a0], y= pos[b0];
  u64 a= ch[x], b= ch[y];
  u32 val= mont::enc(x0);
  S3 f{mont::R1, 0, 0};
  while(comp_hld::head_of(a) != comp_hld::head_of(b)) {
   if(comp_hld::head_of(a) > comp_hld::head_of(b)) val= up_prefix(x, val), x= comp_hld::up_of(a), a= ch[x];
   else f= down_prefix(y, f), y= comp_hld::up_of(b), b= ch[y];
  }
  if(x >= y) val= up_range(y, x, val);
  else f= down_range(x, y, f);
  return mont::dec(down(val, f));
 }
};
}
