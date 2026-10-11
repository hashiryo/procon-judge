#pragma once
// wavelet matrix を書き比べるための核。yosupo の range-kth-smallest、static-range-frequency、
// static-range-sum-with-upper-bound、rectangle-sum の wm_* が共有する。
//
// 値を 0 以上 sigma 未満の順位に詰め、上の bit から段ごとに、その bit が 0 のものを前、1 のものを後ろへ安定に分ける。
// 段 h の位置 i は、bit が 0 なら次の段の i - rank1(i) へ、1 なら zero[h] + rank1(i) へ移る (rank1(i) は段 h の [0, i)
// の 1 の数、zero[h] は段 h の 0 の数)。Sum = true なら、段ごとに分けたあとの並びの重みの累積も持ち、x 未満の値の
// 重みの和を引ける。bit 列の持ち方を 2 通り置く。
//   Sep: 64 bit の語の列と、語ごとのそれより前の 1 の数 (32 bit) の列を別々に持つ。今の Library の形に近い。
//   B32: 32 bit の bit と、そのブロックより前の 1 の数 (32 bit) を 1 つの 64 bit の語に詰める。rank は 1 回の読み出し。
// 値を詰めるところは、(値, 添字) を 64 bit に詰めて 10 bit ずつ 3 回の基数ソートで並べる (値は 2^30 未満)。
// 下りる段の選び方は分岐なし (空の asm を通したマスクの算術) で書く。Sum の累積は、使わない段でも読んで捨てる (次の段の位置は
// bit 列だけで決まるので、累積の読み出しは待たない)。2 MB 以上の配列は Linux では 2 MB 境界の mmap に置いて
// MADV_HUGEPAGE を頼む。設計は algo-notes の notes/wavelet-matrix.md。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace wavelet {
using u32= unsigned;
using u64= unsigned long long;
using i64= long long;
// 2 MB 以上を Linux では 2 MB 境界の mmap に置いて MADV_HUGEPAGE を頼む allocator (NeoLibrary の OrderedSet と同じ形)。
template <class T> struct HugeAlloc {
 using value_type= T;
 static constexpr std::size_t H= std::size_t(1) << 21, P= 4096;
 HugeAlloc()= default;
 template <class U> HugeAlloc(const HugeAlloc<U>&) {}
 T* allocate(std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   const std::size_t bytes= (n * sizeof(T) + P - 1) & ~(P - 1);
   void* m= mmap(nullptr, bytes + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if(m == MAP_FAILED) throw std::bad_alloc();
   char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
   if(a != p) munmap(p, a - p);
   munmap(a + bytes, p + H - a);
   madvise(a, bytes, MADV_HUGEPAGE);
   return reinterpret_cast<T*>(a);
  }
#endif
  return std::allocator<T>().allocate(n);
 }
 void deallocate(T* p, std::size_t n) {
#ifdef __linux__
  if(n * sizeof(T) >= H) {
   munmap(p, (n * sizeof(T) + P - 1) & ~(P - 1));
   return;
  }
#endif
  std::allocator<T>().deallocate(p, n);
 }
 template <class U> bool operator==(const HugeAlloc<U>&) const { return true; }
 template <class U> bool operator!=(const HugeAlloc<U>&) const { return false; }
};
template <class T> using Vec= std::vector<T, HugeAlloc<T>>;
// a を (値, 添字) の昇順に並べ、上位 32 bit に値、下位 32 bit に添字を詰めて返す。a は 0 以上 2^30 未満。
inline Vec<u64> sorted_keys(const std::vector<int>& a) {
 const int n= a.size();
 Vec<u64> p(n), q(n);
 for(int i= 0; i < n; ++i) p[i]= u64(a[i]) << 32 | u32(i);
 for(int s= 32; s < 62; s+= 10) {
  u32 cnt[1025]= {};
  for(u64 k: p) ++cnt[(k >> s & 1023) + 1];
  for(int b= 0; b < 1024; ++b) cnt[b + 1]+= cnt[b];
  for(u64 k: p) q[cnt[k >> s & 1023]++]= k;
  p.swap(q);
 }
 return p;
}
// 値を順位に詰めたもの。id[i] は a[i] の順位、vals は異なる値の昇順。
struct Compressed {
 std::vector<u32> id;
 std::vector<int> vals;
};
inline Compressed compress(const std::vector<int>& a) {
 const int n= a.size();
 Compressed c;
 c.id.resize(n);
 const Vec<u64> k= sorted_keys(a);
 for(int j= 0; j < n; ++j) {
  const int v= int(k[j] >> 32);
  if(c.vals.empty() || c.vals.back() != v) c.vals.push_back(v);
  c.id[u32(k[j])]= c.vals.size() - 1;
 }
 return c;
}
// 昇順の v[0, n) のうち x 未満のものの数 (分岐なしの二分探索)。
inline int lower(const int* v, int n, int x) {
 if(n == 0) return 0;
 const int* b= v;
 while(n > 1) {
  const int half= n / 2;
  b= b[half] < x ? b + half : b;
  n-= half;
 }
 return int(b - v) + (*b < x);
}
inline int lower(const std::vector<int>& v, int x) { return lower(v.data(), v.size(), x); }
// c が真なら全部の bit が 1、偽なら 0 のマスク。GCC はマスクの算術で書いた選択も、同じ条件の選択をまとめて分岐に
// 戻す (kth の l と r で起きた) ので、空の asm を通して値の出どころを見えなくする。
inline int mask(bool c) {
 int m= -int(c);
 asm("" : "+r"(m));
 return m;
}
// m のマスクで a と b から選ぶ。
inline int sel(int m, int a, int b) { return b ^ ((a ^ b) & m); }
// 昇順の v[0, n) のうち x 未満のものの数。lower と同じ二分探索を、マスクを通して分岐なしに固定したもの (lower は
// GCC 15 だと cmov、clang だと分岐になり、frequency の pos が x64-clang で 2.5 倍遅かった)。
inline int lower_bl(const int* v, int n, int x) {
 if(n == 0) return 0;
 int b= 0;
 while(n > 1) {
  const int half= n / 2;
  b+= half & mask(v[b + half] < x);
  n-= half;
 }
 return b + (v[b] < x);
}
// 昇順の v[0, n) のうち x 未満のものの数。lower の条件式は GCC 15 だと cmov になるので、clang にも
// __builtin_unpredictable で cmov を選ばせる。2 回目の lower_bl (マスクの形) は、GCC で cmov より 1 段の待ちが長く、
// x64-gcc の pos_bl は pos の 1.5 倍かかった。
inline int lower_cm(const int* v, int n, int x) {
 if(n == 0) return 0;
 const int* b= v;
 while(n > 1) {
  const int half= n / 2;
#ifdef __clang__
  b= __builtin_unpredictable(b[half] < x) ? b + half : b;
#else
  b= b[half] < x ? b + half : b;
#endif
  n-= half;
 }
 return int(b - v) + (*b < x);
}
// 昇順の v (重複してよい、値は 0 以上 2^30 未満) のうち x 未満のものの数を引く表。値を上位 16 bit で 65536 個の区間に
// 分け、区間ごとにそれより前の数を持つ。引くときは区間の中だけを lower_bl で探す (区間の大きさは平均 n / 65536)。
struct Table {
 static constexpr int B= 16, SH= 30 - B;
 const int* v= nullptr;
 std::vector<int> t;  // t[k] は v のうち (x >> SH) < k のものの数
 Table()= default;
 explicit Table(const std::vector<int>& v_): v(v_.data()), t((1 << B) + 1) {
  const int n= v_.size();
  for(int i= 0, k= 0; k <= (1 << B); ++k) {
   while(i < n && (v_[i] >> SH) < k) ++i;
   t[k]= i;
  }
 }
 // x は 0 以上 2^30 未満。
 int lower(int x) const {
  const int k= x >> SH, lo= t[k];
  return lo + lower_bl(v + lo, t[k + 1] - lo, x);
 }
 // 区間の中を lower_cm で探す形。
 int lower_cm(int x) const {
  const int k= x >> SH, lo= t[k];
  return lo + wavelet::lower_cm(v + lo, t[k + 1] - lo, x);
 }
};
// 段ごとの bit 列 (全段を 1 本の配列に段の順で並べる)。
struct Sep {
 int nw= 0;  // 1 段の語の数
 Vec<u64> w;
 Vec<u32> c;
 void init(int n, int lg) { nw= n / 64 + 1, w.assign(std::size_t(nw) * lg, 0), c.assign(std::size_t(nw) * lg, 0); }
 // 段 h の bit を、cur[i] の bit h から作る。1 の数を返す。
 int fill(int h, const u32* cur, int n) {
  u64* W= w.data() + std::size_t(h) * nw;
  u32* C= c.data() + std::size_t(h) * nw;
  for(int j= 0; j * 64 < n; ++j) {
   const int m= std::min(64, n - j * 64);
   u64 x= 0;
   for(int t= 0; t < m; ++t) x|= u64(cur[j * 64 + t] >> h & 1) << t;
   W[j]= x;
  }
  u32 s= 0;
  for(int j= 0; j < nw; ++j) C[j]= s, s+= __builtin_popcountll(W[j]);
  return s;
 }
 int rank1(int h, int i) const {
  const std::size_t o= std::size_t(h) * nw + (i >> 6);
  return c[o] + __builtin_popcountll(w[o] & ((u64(1) << (i & 63)) - 1));
 }
};
struct B32 {
 int nb= 0;  // 1 段のブロックの数
 Vec<u64> b;  // 下位 32 bit が bit、上位 32 bit がそのブロックより前の 1 の数
 void init(int n, int lg) { nb= n / 32 + 1, b.assign(std::size_t(nb) * lg, 0); }
 int fill(int h, const u32* cur, int n) {
  u64* B= b.data() + std::size_t(h) * nb;
  u32 s= 0;
  for(int j= 0; j < nb; ++j) {
   const int m= std::max(0, std::min(32, n - j * 32));
   u32 x= 0;
   for(int t= 0; t < m; ++t) x|= (cur[j * 32 + t] >> h & 1) << t;
   B[j]= u64(s) << 32 | x;
   s+= __builtin_popcount(x);
  }
  return s;
 }
 int rank1(int h, int i) const {
  const u64 x= b[std::size_t(h) * nb + (i >> 5)];
  return int(x >> 32) + __builtin_popcount(u32(x) & ((u32(1) << (i & 31)) - 1));
 }
};
template <class BV, bool Sum= false> struct WaveletMatrix {
 int n= 0, lg= 1;
 u32 sigma= 0;
 BV bv;
 std::vector<int> zero;  // 段 h の 0 の数
 Vec<i64> sums;          // Sum のとき。段 h で分けたあとの並びの重みの累積 (段ごとに n + 1 個)。最後の段の後ろに元の並びの累積
 WaveletMatrix()= default;
 // v は 0 以上 sigma 未満。Sum のときは w に重みを渡す。
 WaveletMatrix(const std::vector<u32>& v, u32 sigma_, const std::vector<i64>* w= nullptr): n(v.size()), sigma(sigma_) {
  while((u32(1) << lg) < sigma) ++lg;
  bv.init(n, lg), zero.assign(lg, 0);
  std::vector<u32> cur(v), nxt(n);
  std::vector<i64> cw, nw;
  if constexpr(Sum) {
   cw= *w, nw.resize(n), sums.assign(std::size_t(lg + 1) * (n + 1), 0);
   i64* S= sums.data() + std::size_t(lg) * (n + 1);
   for(int i= 0; i < n; ++i) S[i + 1]= S[i] + cw[i];
  }
  for(int h= lg - 1; h >= 0; --h) {
   const int z= n - bv.fill(h, cur.data(), n);
   zero[h]= z;
   int i0= 0, i1= z;
   for(int i= 0; i < n; ++i) {
    const u32 b= cur[i] >> h & 1;
    const int j= b ? i1 : i0;
    nxt[j]= cur[i];
    if constexpr(Sum) nw[j]= cw[i];
    i1+= b, i0+= b ^ 1;
   }
   cur.swap(nxt);
   if constexpr(Sum) {
    cw.swap(nw);
    i64* S= sums.data() + std::size_t(h) * (n + 1);
    for(int i= 0; i < n; ++i) S[i + 1]= S[i] + cw[i];
   }
  }
 }
 // [l, r) の小さい方から k 番目の値の順位。0 <= k < r - l。
 u32 kth(int l, int r, int k) const {
  u32 res= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), z= (r - l) - (r1 - l1), m= mask(k >= z);
   k-= z & m;
   res|= u32(m & 1) << h;
   l= sel(m, zero[h] + l1, l - l1);
   r= sel(m, zero[h] + r1, r - r1);
  }
  return res;
 }
 // [l, r) の順位 x の値の数。x < sigma。
 int count_eq(int l, int r, u32 x) const {
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), m= mask(x >> h & 1);
   l= sel(m, zero[h] + l1, l - l1);
   r= sel(m, zero[h] + r1, r - r1);
  }
  return r - l;
 }
 // [l, r) の順位 x 未満の値の数。x は 0 以上 sigma 以下。
 int count_less(int l, int r, u32 x) const {
  if(x >= sigma) return r - l;
  int c= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), m= mask(x >> h & 1);
   c+= ((r - r1) - (l - l1)) & m;
   l= sel(m, zero[h] + l1, l - l1);
   r= sel(m, zero[h] + r1, r - r1);
  }
  return c;
 }
 // [l, r) の順位 x 未満の値の数と重みの和。x は 0 以上 sigma 以下。Sum のときだけ。
 std::pair<int, i64> count_sum_less(int l, int r, u32 x) const {
  static_assert(Sum);
  if(x >= sigma) {
   const i64* S= sums.data() + std::size_t(lg) * (n + 1);
   return {r - l, S[r] - S[l]};
  }
  int c= 0;
  i64 s= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), l0= l - l1, r0= r - r1, m= mask(x >> h & 1);
   const i64* S= sums.data() + std::size_t(h) * (n + 1);
   c+= (r0 - l0) & m, s+= (S[r0] - S[l0]) & i64(m);
   l= sel(m, zero[h] + l1, l0);
   r= sel(m, zero[h] + r1, r0);
  }
  return {c, s};
 }
 // count_sum_less と同じ値を返す。累積は x の bit が 1 の段だけ読む (分岐で飛ばす)。主記憶への読み出しは半分に
 // なるが、x の bit で分岐する。
 std::pair<int, i64> count_sum_less_br(int l, int r, u32 x) const {
  static_assert(Sum);
  if(x >= sigma) {
   const i64* S= sums.data() + std::size_t(lg) * (n + 1);
   return {r - l, S[r] - S[l]};
  }
  int c= 0;
  i64 s= 0;
  for(int h= lg - 1; h >= 0; --h) {
   const int l1= bv.rank1(h, l), r1= bv.rank1(h, r), l0= l - l1, r0= r - r1;
   if(x >> h & 1) {
    const i64* S= sums.data() + std::size_t(h) * (n + 1);
    c+= r0 - l0, s+= S[r0] - S[l0];
    l= zero[h] + l1, r= zero[h] + r1;
   } else l= l0, r= r0;
  }
  return {c, s};
 }
};
}
