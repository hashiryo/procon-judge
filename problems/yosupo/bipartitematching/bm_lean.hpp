#pragma once
// 診断用。NeoLibrary 5438e82 の BipartiteMatching (neo/combinatorial_optimization/BipartiteMatching.hpp) を写し、次を変えたもの。
// cf-1630-f (procon-judge の problems/cf/1630-f/bm_lean.hpp に同じ写しがある) で lib-neo が今の Library より遅い分の内訳を
// 分けるために作った形を、疎で難しいケースのこの問題でも測る。
// - 辺の列を写して持たず、左の CSR に辺の番号を並べて持つ。matching() は、相手の右の頂点を左の CSR で探して辺の番号を引く。
// - HP が true なら、2 MB 以上の配列 (左右の CSR と辺の番号) を、Linux では 2 MB 境界の mmap に置いて MADV_HUGEPAGE を頼む。
//   この問題の辺は 20 万本なので、どの配列も 2 MB に届かず効かない。
// - KS が true なら最初のマッチングを Karp-Sipser で作る (NeoLibrary と同じ)。false なら、左の頂点を番号の順に見て空いている
//   最初の隣と組ませる貪欲で作る。どちらも、そのあとの push-relabel は NeoLibrary と同じ。
// 使うのは構築、size()、mate()、matching() だけなので、Dulmage-Mendelsohn 分解などは写していない。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace bm_lean {
template <class T, bool HP> struct BigAlloc {
 using value_type= T;
 static constexpr std::size_t H= std::size_t(1) << 21;
 template <class U> struct rebind {
  using other= BigAlloc<U, HP>;
 };
 BigAlloc()= default;
 template <class U> BigAlloc(const BigAlloc<U, HP>&) {}
 T* allocate(std::size_t n) {
#ifdef __linux__
  if(HP && n * sizeof(T) >= H) {
   const std::size_t len= (n * sizeof(T) + H - 1) & ~(H - 1);
   void* m= mmap(nullptr, len + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m == MAP_FAILED) throw std::bad_alloc();
   char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
   if(a != p) munmap(p, a - p);
   munmap(a + len, p + H - a);
   madvise(a, len, MADV_HUGEPAGE);
   return reinterpret_cast<T*>(a);
  }
#endif
  return static_cast<T*>(::operator new(n * sizeof(T)));
 }
 void deallocate(T* p, [[maybe_unused]] std::size_t n) {
#ifdef __linux__
  if(HP && n * sizeof(T) >= H) {
   munmap(p, (n * sizeof(T) + H - 1) & ~(H - 1));
   return;
  }
#endif
  ::operator delete(p);
 }
 template <class U> bool operator==(const BigAlloc<U, HP>&) const { return true; }
};
template <bool KS, bool HP> class BipartiteMatching {
 using Vec= std::vector<int, BigAlloc<int, HP>>;
public:
 BipartiteMatching(int l, int r, const std::vector<std::array<int, 2>>& edges): L(l), R(r) { solve(edges); }
 int size() const { return sz; }
 int mate(int v) const { return v < L ? (ml[v] < 0 ? -1 : L + ml[v]) : mr[v - L]; }
 std::vector<int> matching() const {
  std::vector<int> ret;
  ret.reserve(sz);
  for(int a= 0; a < L; ++a)
   if(const int b= ml[a]; b >= 0)
    for(int k= sl[a];; ++k)
     if(al[k] == b) {
      ret.push_back(ei[k]);
      break;
     }
  std::sort(ret.begin(), ret.end());
  return ret;
 }
private:
 int L, R, sz= 0;
 std::vector<int> sl, sr, ml, mr;
 Vec al, ei, ar;
 void solve(const std::vector<std::array<int, 2>>& es) {
  const int M= es.size(), INF= 2 * R;
  sl.assign(L + 1, 0), al.resize(M), ei.resize(M), sr.assign(R + 1, 0), ar.resize(M);
  for(auto& [a, b]: es) ++sl[a], ++sr[b];
  for(int i= 0; i < L; ++i) sl[i + 1]+= sl[i];
  for(int i= 0; i < R; ++i) sr[i + 1]+= sr[i];
  // 後ろから詰めるので、左の頂点ごとの並びの中で辺の番号は昇順になる。
  for(int i= M; i--;) {
   const auto [a, b]= es[i];
   const int k= --sl[a];
   al[k]= b, ei[k]= i, ar[--sr[b]]= a;
  }
  ml.assign(L, -1), mr.assign(R, -1);
  if constexpr(KS) {
   std::vector<int> dl(L), dr(R), stk(L + R);
   int sp= 0;
   for(int l= 0; l < L; ++l)
    if((dl[l]= sl[l + 1] - sl[l]) == 1) stk[sp++]= l;
   for(int r= 0; r < R; ++r)
    if((dr[r]= sr[r + 1] - sr[r]) == 1) stk[sp++]= L + r;
   auto take= [&](int l, int r) {
    ml[l]= r, mr[r]= l;
    for(int k= sl[l]; k < sl[l + 1]; ++k)
     if(const int r2= al[k]; mr[r2] < 0 && --dr[r2] == 1) stk[sp++]= L + r2;
    for(int k= sr[r]; k < sr[r + 1]; ++k)
     if(const int l2= ar[k]; ml[l2] < 0 && --dl[l2] == 1) stk[sp++]= l2;
   };
   for(int cur= 0;;) {
    while(sp) {
     const int x= stk[--sp];
     if(x < L) {
      if(ml[x] >= 0) continue;
      for(int k= sl[x]; k < sl[x + 1]; ++k)
       if(const int r= al[k]; mr[r] < 0) {
        take(x, r);
        break;
       }
     } else {
      const int r= x - L;
      if(mr[r] >= 0) continue;
      for(int k= sr[r]; k < sr[r + 1]; ++k)
       if(const int l= ar[k]; ml[l] < 0) {
        take(l, r);
        break;
       }
     }
    }
    while(cur < L && (ml[cur] >= 0 || dl[cur] == 0)) ++cur;
    if(cur == L) break;
    for(int k= sl[cur]; k < sl[cur + 1]; ++k)
     if(const int r= al[k]; mr[r] < 0) {
      take(cur, r);
      break;
     }
   }
  } else {
   for(int l= 0; l < L; ++l)
    for(int k= sl[l]; k < sl[l + 1]; ++k)
     if(const int r= al[k]; mr[r] < 0) {
      ml[l]= r, mr[r]= l;
      break;
     }
  }
  struct Rv {
   int psi, mate;
  };
  std::vector<Rv> rv(R);
  std::vector<int> q(L), bq(R);
  for(int r= 0; r < R; ++r) rv[r].mate= mr[r];
  auto global_relabel= [&] {
   int h= 0, t= 0;
   for(int r= 0; r < R; ++r)
    if(rv[r].mate < 0) rv[r].psi= 0, bq[t++]= r;
    else rv[r].psi= INF;
   while(h < t) {
    const int r= bq[h++], d= rv[r].psi + 2;
    for(int k= sr[r]; k < sr[r + 1]; ++k)
     if(const int r2= ml[ar[k]]; r2 >= 0 && rv[r2].psi == INF) rv[r2].psi= d, bq[t++]= r2;
   }
  };
  global_relabel();
  int qh= 0, qn= 0;
  for(int l= 0; l < L; ++l)
   if(ml[l] < 0) q[qn++]= l;
  const int freq= std::max(1, (L + R) / 2);
  for(int cnt= 0; qn;) {
   const int u= q[qh];
   if(++qh == L) qh= 0;
   --qn;
   int m1= INF, m2= INF, r1= -1;
   for(int k= sl[u]; k < sl[u + 1]; ++k) {
    const int r= al[k], p= rv[r].psi;
    if(p < m1) m2= m1, m1= p, r1= r;
    else if(p < m2) m2= p;
   }
   if(m1 >= INF) continue;
   const int w= rv[r1].mate;
   ml[u]= r1, rv[r1]= {std::min(m2 + 2, INF), u};
   if(w >= 0) {
    ml[w]= -1;
    int t= qh + qn;
    if(t >= L) t-= L;
    q[t]= w, ++qn;
   }
   if(++cnt == freq) cnt= 0, global_relabel();
  }
  for(int r= 0; r < R; ++r) mr[r]= rv[r].mate;
  for(int l= 0; l < L; ++l) sz+= ml[l] >= 0;
 }
};
}  // namespace bm_lean
