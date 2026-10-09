#pragma once
// msd16_net32_fused の、振り分けて並べる道の返り値の xs を huge page にする版。xs は容量 n で確保だけしてから、2 MB 境界に揃った中の
// 部分に huge page を頼み、ページの境界に揃った中の部分をまとめて用意させる (self-sort-compress-u32 の pack_msd16_net32_fused_ph と同じ
// 手)。i64 では一様なケースの xs が 8 MB になり、4 KB のページのままだと、値を足していくうちにページフォールトが 2000 回ほど起きる。
// ほかは msd16_net32_fused と同じ。以下は msd16_net32_fused の説明。
// 値と添字を別の作業用の配列へ値の上の桁で振り分け、塊の中を u32 の鍵の AVX2 のネットワークで並べて (self-sort-i64 の msd16_net32 の
// 手)、並べたその場で順位を数えて書く (self-sort-compress-u32 の pack_msd16_net32_fused の手)。値と添字は合わせて 84 bit で u64 に
// 収まらないので、値 (8 byte) と添字 (4 byte) を 2 本の配列の同じ位置に書く。塊の中では、鍵 (最上位 bit を反転した値) から base を
// 引いた値の桁とそれより上の bit がそろっている。桁より下の bit のうち上の 26 bit までを上に、塊の中の位置を下の 6 bit に詰めた u32
// を鍵にして並べ、並べた順に値と添字をスタックの 64 個の領域に集める。26 bit が等しい隣どうしがあれば、そこだけ挿入ソートで値の順に
// 直す。それから隣どうしの値を比べて順位を数え、a の添字の位置に書く。塊は値の順に並んでいて、違う塊の値は必ず違うので、順位は
// 塊をまたいで数え続ければよい。桁の選び方は self-sort-i64 の msd16_net32 と同じ。桁より下の bit が全要素で同じなら、空でない塊
// ごとに値が 1 つなので、振り分けずに、塊の番号から順位を引く表で書き換える。64 個を超える塊は、値と添字の組を std::sort で並べて
// から走査する。値の種類が 1024 個以下なら、ハッシュ表で順位を引く (self-sort-compress-u32 の _hash の手)。塊ごとに通る関数は
// always_inline にする。作業用の配列は huge page にする。
#ifdef USE_SIMDE
#include <simde/x86/avx2.h>
#else
#include <immintrin.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>
namespace compress_msd16_net32_fused_ph {
using T= long long;
using u32= unsigned;
using u64= unsigned long long;
// 桁を取り出すときの鍵。最上位 bit を反転して、鍵の符号なしの順が値の順になるようにする。
inline u64 key(T x) { return u64(x) ^ (u64(1) << 63); }
struct FreeDeleter {
 void operator()(void* p) const { std::free(p); }
};
// 2 MB 境界で確保し、huge page を頼んでから、触る前にまとめて用意させる。
template <class U> inline std::unique_ptr<U, FreeDeleter> alloc_huge(size_t n) {
 constexpr size_t H= size_t(1) << 21;
 const size_t bytes= (n * sizeof(U) + H - 1) & ~(H - 1);
 void* p= std::aligned_alloc(H, bytes);
#ifdef __linux__
 madvise(p, bytes, MADV_HUGEPAGE);
 madvise(p, bytes, 23);  // MADV_POPULATE_WRITE
#endif
 return std::unique_ptr<U, FreeDeleter>(static_cast<U*>(p));
}
// u32 の鍵のソーティングネットワーク (self-sort-u32 の msd16_net と同じ)。stepD<MASK> は距離 D の lane どうしを比べ、MASK の bit が
// 立つ lane に大きい方を置く。
namespace net32 {
inline __m256i vmin(__m256i a, __m256i b) { return _mm256_min_epu32(a, b); }
inline __m256i vmax(__m256i a, __m256i b) { return _mm256_max_epu32(a, b); }
template <int MASK> inline __m256i step1(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0xB1);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step2(__m256i v) {
 __m256i q= _mm256_shuffle_epi32(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
template <int MASK> inline __m256i step4(__m256i v) {
 __m256i q= _mm256_permute4x64_epi64(v, 0x4E);
 return _mm256_blend_epi32(vmin(v, q), vmax(v, q), MASK);
}
// bitonic な 8 個を昇順にする。
inline __m256i merge8(__m256i v) { return step1<0xAA>(step2<0xCC>(step4<0xF0>(v))); }
// 任意の 8 個を昇順にする。2 個ずつ昇順と降順、4 個ずつ昇順と降順に並べて bitonic にしてから merge8。
inline __m256i sort8(__m256i v) { return merge8(step1<0x5A>(step2<0x3C>(step1<0x66>(v)))); }
inline __m256i reverse8(__m256i v) { return _mm256_permutevar8x32_epi32(v, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }
// a, b を並べた 16 個が bitonic なら昇順にする。
[[gnu::always_inline]] inline void merge16(__m256i& a, __m256i& b) {
 __m256i l= vmin(a, b), h= vmax(a, b);
 a= merge8(l), b= merge8(h);
}
[[gnu::always_inline]] inline void sort16(__m256i& a, __m256i& b) {
 a= sort8(a), b= reverse8(sort8(b));
 merge16(a, b);
}
[[gnu::always_inline]] inline void sort32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 sort16(a, b), sort16(c, d);
 // 後ろの 16 個を逆順にして (reverse8(d), reverse8(c)) とつなぐと 32 個が bitonic になる。
 __m256i rc= reverse8(d), rd= reverse8(c);
 __m256i l0= vmin(a, rc), h0= vmax(a, rc), l1= vmin(b, rd), h1= vmax(b, rd);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// a, b, c, d を並べた 32 個が bitonic なら昇順にする。
[[gnu::always_inline]] inline void merge32(__m256i& a, __m256i& b, __m256i& c, __m256i& d) {
 __m256i l0= vmin(a, c), h0= vmax(a, c), l1= vmin(b, d), h1= vmax(b, d);
 merge16(l0, l1), merge16(h0, h1);
 a= l0, b= l1, c= h0, d= h1;
}
// 8 本のベクタの 64 個を昇順にする。前後の 32 個を並べ、後ろを逆順にしてつないだ bitonic な 64 個を merge する。
[[gnu::always_inline]] inline void sort64(__m256i* v) {
 sort32(v[0], v[1], v[2], v[3]), sort32(v[4], v[5], v[6], v[7]);
 const __m256i r4= reverse8(v[7]), r5= reverse8(v[6]), r6= reverse8(v[5]), r7= reverse8(v[4]);
 __m256i l0= vmin(v[0], r4), h0= vmax(v[0], r4), l1= vmin(v[1], r5), h1= vmax(v[1], r5);
 __m256i l2= vmin(v[2], r6), h2= vmax(v[2], r6), l3= vmin(v[3], r7), h3= vmax(v[3], r7);
 merge32(l0, l1, l2, l3), merge32(h0, h1, h2, h3);
 v[0]= l0, v[1]= l1, v[2]= l2, v[3]= l3, v[4]= h0, v[5]= h1, v[6]= h2, v[7]= h3;
}
}  // namespace net32
// 塊の鍵の作り方。鍵 (key(x) - base) の bit [ks, ks + rk) を上に、塊の中の位置を下の 6 bit に詰める (rk <= 26)。
struct KeyParams {
 __m256i flip, base, mask;  // flip は符号付きのとき最上位 bit、mask は下の rk bit (64 bit の lane ごと)
 __m128i ks;
};
// 塊 s[0, m) の 8t 番目からの 8 個の鍵を作る。値を 4 個ずつ 2 回読み、64 bit のまま鍵の bit を取り出してから、下の 32 bit を元の順に
// 集める。m 個より後ろの lane は最大値で埋める (読むのは s[0, 8t + 8) で、塊の後ろの要素を読んでも鍵には使わない)。
[[gnu::always_inline]] inline __m256i make_keys(const T* s, int m, int t, const KeyParams& kp) {
 const int at= 8 * t;
 const auto part= [&](const T* p) {
  const __m256i v= _mm256_sub_epi64(_mm256_xor_si256(_mm256_loadu_si256((const __m256i*)p), kp.flip), kp.base);
  return _mm256_castsi256_ps(_mm256_and_si256(_mm256_srl_epi64(v, kp.ks), kp.mask));
 };
 // shuffle_ps で 128 bit の中ごとに偶数番目の 32 bit を集めると (y0 y1 y4 y5 | y2 y3 y6 y7) になるので、64 bit 単位で並べ直す。
 const __m256i y= _mm256_permute4x64_epi64(_mm256_castps_si256(_mm256_shuffle_ps(part(s + at), part(s + at + 4), 0x88)), 0xD8);
 const __m256i iota= _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
 const __m256i k= _mm256_or_si256(_mm256_slli_epi32(y, 6), _mm256_add_epi32(iota, _mm256_set1_epi32(at)));
 if(m - at >= 8) return k;
 return _mm256_blendv_epi8(_mm256_set1_epi32(-1), k, _mm256_cmpgt_epi32(_mm256_set1_epi32(m - at), iota));
}
// 並べた鍵のベクタ v[0, nv) の先頭 m 個に、上の 26 bit が等しい隣どうしがあるか。1 lane ずらした鍵と比べる。
[[gnu::always_inline]] inline bool has_tie(const __m256i* v, int nv, int m) {
 const __m256i up= _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), last= _mm256_set1_epi32(7);
 __m256i carry= _mm256_set1_epi32(-1);  // 先頭の要素の前には、どの鍵とも等しくならない値を置く
 for(int t= 0; t < nv; ++t) {
  const int rem= m - 8 * t;
  if(rem <= 0) break;
  const __m256i p= _mm256_srli_epi32(v[t], 6);
  const __m256i q= _mm256_blend_epi32(_mm256_permutevar8x32_epi32(p, up), carry, 0x01);
  int bits= _mm256_movemask_ps(_mm256_castsi256_ps(_mm256_cmpeq_epi32(p, q)));
  if(rem < 8) bits&= (1 << rem) - 1;
  if(bits) return true;
  carry= _mm256_permutevar8x32_epi32(p, last);
 }
 return false;
}
// 並べた値 d と添字 di の鍵 k[0, m) の上の 26 bit が等しい並びごとに、挿入ソートで値の順に直す (添字も一緒に動かす)。
inline void fix_ties(const u32* k, int m, T* d, u32* di) {
 for(int i= 0; i < m;) {
  int j= i + 1;
  while(j < m && (k[j] ^ k[i]) < 64) ++j;
  for(int x= i + 1; x < j; ++x) {
   const T y= d[x];
   const u32 yi= di[x];
   int z= x;
   for(; z > i && y < d[z - 1]; --z) d[z]= d[z - 1], di[z]= di[z - 1];
   d[z]= y, di[z]= yi;
  }
  i= j;
 }
}
// 値の順に並んだ値 d と添字 di の [0, m) (m >= 1) を走査し、値が変わるたびに順位を 1 増やして xs に足し、a の添字の位置に順位を
// 書く。rk はそれまでの値の種類数 (次に付ける順位)。最初の要素は、前の塊と値が必ず違うので新しい値になる。
template <class D, class I> [[gnu::always_inline]] inline void rank_run(const D& d, const I& di, size_t m, T* a, std::vector<T>& xs, u32& rk) {
 T prev= d(0);
 xs.push_back(prev), ++rk;
 a[di(0)]= T(rk - 1);
 for(size_t i= 1; i < m; ++i) {
  const T v= d(i);
  if(v != prev) xs.push_back(v), prev= v, ++rk;
  a[di(i)]= T(rk - 1);
 }
}
// 塊 (値 s、添字 si) の [0, m) (2 <= m <= 64) を値の順に並べ、順位を数えて a に書く。fix は鍵が桁より下の bit を全部は持たないとき。
[[gnu::always_inline]] inline void rank_bucket(const T* s, const u32* si, int m, const KeyParams& kp, bool fix, T* a, std::vector<T>& xs, u32& rk) {
 alignas(32) u32 k[64];
 __m256i v[8];
 const __m256i pad= _mm256_set1_epi32(-1);
 int nv;
 if(m <= 8) {
  nv= 1, v[0]= net32::sort8(make_keys(s, m, 0, kp));
 } else if(m <= 16) {
  nv= 2, v[0]= make_keys(s, m, 0, kp), v[1]= make_keys(s, m, 1, kp);
  net32::sort16(v[0], v[1]);
 } else if(m <= 32) {
  nv= 4;
  for(int t= 0; t < 4; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, kp) : pad;
  net32::sort32(v[0], v[1], v[2], v[3]);
 } else {
  nv= 8;
  for(int t= 0; t < 8; ++t) v[t]= 8 * t < m ? make_keys(s, m, t, kp) : pad;
  net32::sort64(v);
 }
 for(int t= 0; t < nv; ++t) _mm256_store_si256((__m256i*)(k + 8 * t), v[t]);
 T d[64];
 u32 di[64];
 for(int i= 0; i < m; ++i) d[i]= s[k[i] & 63], di[i]= si[k[i] & 63];
 if(fix && has_tie(v, nv, m)) fix_ties(k, m, d, di);
 rank_run([&](size_t i) { return d[i]; }, [&](size_t i) { return di[i]; }, size_t(m), a, xs, rk);
}
// 値と添字の組 p[0, m) を std::sort で並べてから走査する (64 個以下の配列と、64 個を超える塊)。
inline void rank_pairs(std::pair<T, u32>* p, size_t m, T* a, std::vector<T>& xs, u32& rk) {
 std::sort(p, p + m);
 rank_run([&](size_t i) { return p[i].first; }, [&](size_t i) { return p[i].second; }, m, a, xs, rk);
}
// 値の種類が 1024 個以下なら、各要素をハッシュ表で引いた順位に書き換えて xs を作り、true を返す。種類が 1024 個を超えた時点で、
// a に手を付けずに false を返す。表は 4096 か所の線形探査で、値 kv と印 tag (0 は空、埋まったら 1、あとで順位 + 1) を別の配列に
// 持つ。消すことはないので、値 x を引くと、x の場所に着くまでに空の場所を通らない。ほとんどの入力ではすぐにやめる道なので、compress
// に展開させない。
[[gnu::noinline]] inline bool few_values(T* a, size_t n, std::vector<T>& xs) {
 constexpr u32 LIM= 1024, B= 12, S= 1u << B;
 std::vector<u64> kv(S);
 std::vector<u32> tag(S);
 const auto slot= [](u64 x) { return u32((x * 0x9E3779B97F4A7C15ull) >> (64 - B)); };
 u32 k= 0;
 for(size_t i= 0; i < n; ++i) {
  const u64 x= u64(a[i]);
  for(u32 h= slot(x);; h= (h + 1) & (S - 1)) {
   if(!tag[h]) {
    if(++k > LIM) return false;
    kv[h]= x, tag[h]= 1;
    break;
   }
   if(kv[h] == x) break;
  }
 }
 xs.reserve(k);
 for(u32 h= 0; h < S; ++h)
  if(tag[h]) xs.push_back(T(kv[h]));
 std::sort(xs.begin(), xs.end());
 for(u32 r= 0; r < k; ++r) {
  const u64 x= u64(xs[r]);
  u32 h= slot(x);
  while(kv[h] != x || !tag[h]) h= (h + 1) & (S - 1);
  tag[h]= r + 1;
 }
 for(size_t i= 0; i < n; ++i) {
  const u64 x= u64(a[i]);
  u32 h= slot(x);
  while(kv[h] != x || !tag[h]) h= (h + 1) & (S - 1);
  a[i]= T(tag[h] - 1);
 }
 return true;
}
// 返り値の xs を、長さ 0、容量 n で作る。確保だけしてから、2 MB 境界に揃った中の部分に huge page を頼み、ページの境界に揃った
// 中の部分をまとめて用意させる。
inline std::vector<T> alloc_xs(size_t n) {
 std::vector<T> xs;
 xs.reserve(n);
#ifdef __linux__
 constexpr uintptr_t H= uintptr_t(1) << 21, P= 4096;
 const uintptr_t s= reinterpret_cast<uintptr_t>(xs.data()), e= s + n * sizeof(T);
 const uintptr_t hs= (s + H - 1) & ~(H - 1), he= e & ~(H - 1);
 if(hs < he) madvise(reinterpret_cast<void*>(hs), he - hs, MADV_HUGEPAGE);
 const uintptr_t ps= (s + P - 1) & ~(P - 1), pe= e & ~(P - 1);
 if(ps < pe) madvise(reinterpret_cast<void*>(ps), pe - ps, 23);  // MADV_POPULATE_WRITE
#endif
 return xs;
}
inline std::vector<T> compress(std::vector<T>& v) {
 const size_t n= v.size();
 T* a= v.data();
 std::vector<T> xs;
 u32 rk= 0;
 if(n <= 64) {
  std::pair<T, u32> p[64];
  for(size_t i= 0; i < n; ++i) p[i]= {a[i], u32(i)};
  xs.reserve(n);
  if(n) rank_pairs(p, n, a, xs, rk);
  return xs;
 }
 u64 o= 0, e= ~0ull, mn= ~0ull, mx= 0;
 for(size_t i= 0; i < n; ++i) {
  const u64 k= key(a[i]);
  o|= k, e&= k, mn= std::min(mn, k), mx= std::max(mx, k);
 }
 const u64 diff= o ^ e;
 if(!diff) {  // 全要素が同じ値
  xs.assign(1, a[0]);
  std::fill(a, a + n, T(0));
  return xs;
 }
 if(n >= (size_t(1) << 14) && few_values(a, n, xs)) return xs;
 // 鍵から base を引いた値の bit [lo, hi] の上から db bit を桁にする。要素によって違う bit の幅と、最大値と最小値の差の幅の狭い方を使う。
 int hi= 63 - __builtin_clzll(diff), lo= __builtin_ctzll(diff);
 u64 base= 0;
 bool by_range= false;
 if(const int wm= 64 - __builtin_clzll(mx - mn); wm < hi - lo + 1) lo= 0, hi= wm - 1, base= mn, by_range= true;
 int db= 1;
 while(db < 16 && (n >> db) > 16) ++db;
 db= std::min(db, hi - lo + 1);
 const int sh= hi - db + 1, r= sh - lo, kr= std::min(r, 26);
 const u64 mk= (u64(1) << db) - 1;
 std::vector<u32> pos(size_t(1) << db);
 for(size_t i= 0; i < n; ++i) ++pos[(key(a[i]) - base) >> sh & mk];
 if(!r) {
  // 桁が値を決めるので、空でない塊ごとに値が 1 つある。最大値と最小値の差で桁を選んだなら鍵は base に桁の値を足したもの、そうでなければ
  // 桁の外の bit (全要素で同じ) に桁の値を入れたもの。
  xs.reserve(std::min(n, pos.size()));
  for(size_t d= 0; d < pos.size(); ++d) {
   const u32 c= pos[d];
   pos[d]= rk;
   if(c) {
    const u64 kd= by_range ? base + (u64(d) << sh) : (e & ~(mk << sh)) | (u64(d) << sh);
    xs.push_back(T(kd ^ (u64(1) << 63))), ++rk;
   }
  }
  for(size_t i= 0; i < n; ++i) a[i]= T(pos[(key(a[i]) - base) >> sh & mk]);
  return xs;
 }
 for(size_t d= 0, s= 0; d < pos.size(); ++d) {
  const u32 t= pos[d];
  pos[d]= u32(s), s+= t;
 }
 auto bb= alloc_huge<T>(n + 64);  // 塊を 8 個ずつ読むので、最後の塊の後ろを 63 個まで読む
 auto ib= alloc_huge<u32>(n);
 T* b= bb.get();
 u32* bi= ib.get();
 for(size_t i= 0; i < n; ++i) {
  const T x= a[i];
  const u32 p= pos[(key(x) - base) >> sh & mk]++;
  b[p]= x, bi[p]= u32(i);
 }
 // 振り分けたあとの pos[d] は塊 d の終わり。
 const KeyParams kp{_mm256_set1_epi64x((long long)(1ull << 63)), _mm256_set1_epi64x((long long)base),
                    _mm256_set1_epi64x((long long)((u64(1) << kr) - 1)), _mm_cvtsi32_si128(sh - kr)};
 const bool fix= r > kr;
 xs= alloc_xs(n);
 for(size_t d= 0, off= 0; d < pos.size(); ++d) {
  const size_t end= pos[d], m= end - off;
  if(m == 1) {
   xs.push_back(b[off]);
   a[bi[off]]= T(rk++);
  } else if(m >= 2 && m <= 64) {
   rank_bucket(b + off, bi + off, int(m), kp, fix, a, xs, rk);
  } else if(m > 64) {
   std::vector<std::pair<T, u32>> p(m);
   for(size_t i= 0; i < m; ++i) p[i]= {b[off + i], bi[off + i]};
   rank_pairs(p.data(), m, a, xs, rk);
  }
  off= end;
 }
 return xs;
}
}  // namespace compress_msd16_net32_fused_ph
inline std::vector<long long> run(std::vector<long long>& a) { return compress_msd16_net32_fused_ph::compress(a); }
