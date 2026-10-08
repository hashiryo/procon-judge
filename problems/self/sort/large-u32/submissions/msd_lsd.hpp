#pragma once
// 上の桁で 1 回振り分けてから、塊ごとに LSD 基数ソートで並べる版 (MSD と LSD の組み合わせ)。最初の走査で全要素の bit or と
// bit and を取り、要素によって違う bit のうち上の 8 bit で作業用の配列へ振り分ける。一様な u32 を 10^6 個なら塊は 256 個で、
// 1 つ約 3900 個 (16 KB)。塊の中は残りの bit を 8 bit 以下の桁に等分して LSD で並べ、作業用の配列と a の同じ位置を行き来する。
// 塊が L1 に載るので、全体をなめる振り分けは最初の 1 回だけで済む。塊の中でも全要素が同じ値になる桁は飛ばし、32 個以下の塊は
// 挿入ソートにする。作業用の配列は 0 で埋めない。256 個以下は std::sort に任せる。
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>
namespace sort_msd_lsd {
using u32= unsigned;
inline void insertion(u32* a, size_t n) {
 for(size_t i= 1; i < n; ++i) {
  u32 x= a[i];
  size_t j= i;
  for(; j > 0 && a[j - 1] > x; --j) a[j]= a[j - 1];
  a[j]= x;
 }
}
// B[0, m) を bit [lo, lo + w) について並べ、結果を A[0, m) に置く。A と B は同じ長さの別の領域。
inline void lsd_block(u32* A, u32* B, size_t m, int lo, int w) {
 if(w <= 0 || m <= 1) return (void)std::memcpy(A, B, m * sizeof(u32));
 if(m <= 32) {
  std::memcpy(A, B, m * sizeof(u32));
  return insertion(A, m);
 }
 const int p= (w + 7) / 8;
 int sh[3]= {}, bits[3]= {};
 for(int d= 0, s= lo; d < p; ++d) {
  bits[d]= w / p + (d < w % p);
  sh[d]= s, s+= bits[d];
 }
 u32 cnt[3][256];
 for(int d= 0; d < p; ++d) std::memset(cnt[d], 0, sizeof(u32) << bits[d]);
 const u32 m0= (1u << bits[0]) - 1, m1= (1u << bits[1]) - 1, m2= (1u << bits[2]) - 1;
 if(p == 1) {
  for(size_t i= 0; i < m; ++i) ++cnt[0][B[i] >> sh[0] & m0];
 } else if(p == 2) {
  for(size_t i= 0; i < m; ++i) {
   u32 x= B[i];
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1];
  }
 } else {
  for(size_t i= 0; i < m; ++i) {
   u32 x= B[i];
   ++cnt[0][x >> sh[0] & m0], ++cnt[1][x >> sh[1] & m1], ++cnt[2][x >> sh[2] & m2];
  }
 }
 // 全要素が同じ値になる桁を除き、残りの桁のヒストグラムを先頭からの位置に直す。
 int use[3], k= 0;
 for(int d= 0; d < p; ++d) {
  const u32 sz= 1u << bits[d];
  bool trivial= false;
  for(u32 i= 0; i < sz; ++i)
   if(cnt[d][i]) {
    trivial= cnt[d][i] == m;
    break;
   }
  if(trivial) continue;
  u32 s= 0;
  for(u32 i= 0; i < sz; ++i) {
   u32 t= cnt[d][i];
   cnt[d][i]= s, s+= t;
  }
  use[k++]= d;
 }
 u32 *src= B, *dst= A;
 for(int j= 0; j < k; ++j) {
  const int d= use[j], s= sh[d];
  const u32 mk= (1u << bits[d]) - 1;
  u32* c= cnt[d];
  for(size_t i= 0; i < m; ++i) {
   u32 x= src[i];
   dst[c[x >> s & mk]++]= x;
  }
  std::swap(src, dst);
 }
 if(src != A) std::memcpy(A, src, m * sizeof(u32));
}
inline void sort(std::vector<u32>& v) {
 const size_t n= v.size();
 if(n <= 256) return std::sort(v.begin(), v.end());
 u32* a= v.data();
 u32 o= 0, e= ~0u;
 for(size_t i= 0; i < n; ++i) o|= a[i], e&= a[i];
 const u32 diff= o ^ e;
 if(!diff) return;  // 全要素が同じ値
 const int lo= __builtin_ctz(diff), hi= 31 - __builtin_clz(diff);
 const int s0= std::max(lo, hi - 7);
 const u32 m0= (1u << (hi - s0 + 1)) - 1;
 size_t cnt[256]= {}, head[256];
 for(size_t i= 0; i < n; ++i) ++cnt[a[i] >> s0 & m0];
 for(size_t d= 0, s= 0; d < 256; ++d) head[d]= s, s+= cnt[d];
 std::unique_ptr<u32[]> buf(new u32[n]);
 u32* b= buf.get();
 {
  size_t pos[256];
  std::memcpy(pos, head, sizeof(pos));
  for(size_t i= 0; i < n; ++i) {
   u32 x= a[i];
   b[pos[x >> s0 & m0]++]= x;
  }
 }
 for(size_t d= 0; d <= m0; ++d)
  if(cnt[d]) lsd_block(a + head[d], b + head[d], cnt[d], lo, s0 - lo);
}
}  // namespace sort_msd_lsd
inline void run(std::vector<unsigned>& a) { sort_msd_lsd::sort(a); }
