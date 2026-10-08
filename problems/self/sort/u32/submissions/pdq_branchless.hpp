#pragma once
// pdqsort (Orson Peters) の分岐しない分割の版を u32 用に写したもの。分割は BlockQuicksort (Edelkamp, Weiß) の形で、
// 64 個ずつ「反対側に行く要素の位置」を比較の結果の足し算で記録してから、まとめて入れ替える。比較の結果で分岐しないので、
// 一様な入力でも分岐予測が外れない。軸は 3 点か 9 点の中央値。軸が直前の区間の境の値と等しければ、等しい要素を左に
// 寄せる分割に切り替える (重複の多い入力向け)。分割が大きく偏ったら要素を混ぜ、偏りが続けばヒープソートに切り替える。
// 分割で入れ替えが起きなかった区間は、ほぼ整列済みとみなして挿入ソートを試す。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
namespace sort_pdq {
using u32= unsigned;
constexpr ptrdiff_t INSERTION_SORT_THRESHOLD= 24;
constexpr ptrdiff_t NINTHER_THRESHOLD= 128;
constexpr ptrdiff_t PARTIAL_INSERTION_SORT_LIMIT= 8;
constexpr size_t BLOCK_SIZE= 64;
constexpr size_t CACHELINE_SIZE= 64;
inline void insertion_sort(u32* begin, u32* end) {
 if(begin == end) return;
 for(u32* cur= begin + 1; cur != end; ++cur) {
  u32 *sift= cur, *sift_1= cur - 1;
  if(*sift < *sift_1) {
   u32 tmp= *sift;
   do *sift--= *sift_1;
   while(sift != begin && tmp < *--sift_1);
   *sift= tmp;
  }
 }
}
// *(begin - 1) が [begin, end) のどの要素以下でもないことを前提に、左端の検査を省く。
inline void unguarded_insertion_sort(u32* begin, u32* end) {
 if(begin == end) return;
 for(u32* cur= begin + 1; cur != end; ++cur) {
  u32 *sift= cur, *sift_1= cur - 1;
  if(*sift < *sift_1) {
   u32 tmp= *sift;
   do *sift--= *sift_1;
   while(tmp < *--sift_1);
   *sift= tmp;
  }
 }
}
// 動かした距離の合計が PARTIAL_INSERTION_SORT_LIMIT を超えたらやめて false を返す。
inline bool partial_insertion_sort(u32* begin, u32* end) {
 if(begin == end) return true;
 ptrdiff_t limit= 0;
 for(u32* cur= begin + 1; cur != end; ++cur) {
  u32 *sift= cur, *sift_1= cur - 1;
  if(*sift < *sift_1) {
   u32 tmp= *sift;
   do *sift--= *sift_1;
   while(sift != begin && tmp < *--sift_1);
   *sift= tmp;
   limit+= cur - sift;
  }
  if(limit > PARTIAL_INSERTION_SORT_LIMIT) return false;
 }
 return true;
}
inline void sort2(u32* a, u32* b) {
 if(*b < *a) std::iter_swap(a, b);
}
inline void sort3(u32* a, u32* b, u32* c) { sort2(a, b), sort2(b, c), sort2(a, b); }
inline unsigned char* align_cacheline(unsigned char* p) {
 std::uintptr_t ip= reinterpret_cast<std::uintptr_t>(p);
 ip= (ip + CACHELINE_SIZE - 1) & -std::uintptr_t(CACHELINE_SIZE);
 return reinterpret_cast<unsigned char*>(ip);
}
inline void swap_offsets(u32* first, u32* last, unsigned char* offsets_l, unsigned char* offsets_r, size_t num, bool use_swaps) {
 if(use_swaps) {
  // 降順の入力で O(n) を保つには、1 組ずつの入れ替えが要る。
  for(size_t i= 0; i < num; ++i) std::iter_swap(first + offsets_l[i], last - offsets_r[i]);
 } else if(num > 0) {
  u32 *l= first + offsets_l[0], *r= last - offsets_r[0];
  u32 tmp= *l;
  *l= *r;
  for(size_t i= 1; i < num; ++i) {
   l= first + offsets_l[i];
   *r= *l;
   r= last - offsets_r[i];
   *l= *r;
  }
  *r= tmp;
 }
}
// 軸 *begin より小さい要素を左に、以上の要素を右に分ける。軸の位置と、入れ替えが 1 度も無かったかを返す。
inline std::pair<u32*, bool> partition_right_branchless(u32* begin, u32* end) {
 const u32 pivot= *begin;
 u32 *first= begin, *last= end;
 while(*++first < pivot);
 if(first - 1 == begin)
  while(first < last && !(*--last < pivot));
 else
  while(!(*--last < pivot));
 const bool already_partitioned= first >= last;
 if(!already_partitioned) {
  std::iter_swap(first, last);
  ++first;
  unsigned char offsets_l_storage[BLOCK_SIZE + CACHELINE_SIZE];
  unsigned char offsets_r_storage[BLOCK_SIZE + CACHELINE_SIZE];
  unsigned char* offsets_l= align_cacheline(offsets_l_storage);
  unsigned char* offsets_r= align_cacheline(offsets_r_storage);
  u32 *offsets_l_base= first, *offsets_r_base= last;
  size_t num_l= 0, num_r= 0, start_l= 0, start_r= 0;
  while(first < last) {
   const size_t num_unknown= last - first;
   const size_t left_split= num_l == 0 ? (num_r == 0 ? num_unknown / 2 : num_unknown) : 0;
   const size_t right_split= num_r == 0 ? (num_unknown - left_split) : 0;
   if(left_split >= BLOCK_SIZE) {
    for(size_t i= 0; i < BLOCK_SIZE;) {
     for(int u= 0; u < 8; ++u) offsets_l[num_l]= i++, num_l+= !(*first < pivot), ++first;
    }
   } else {
    for(size_t i= 0; i < left_split;) offsets_l[num_l]= i++, num_l+= !(*first < pivot), ++first;
   }
   if(right_split >= BLOCK_SIZE) {
    for(size_t i= 0; i < BLOCK_SIZE;) {
     for(int u= 0; u < 8; ++u) offsets_r[num_r]= ++i, num_r+= *--last < pivot;
    }
   } else {
    for(size_t i= 0; i < right_split;) offsets_r[num_r]= ++i, num_r+= *--last < pivot;
   }
   const size_t num= std::min(num_l, num_r);
   swap_offsets(offsets_l_base, offsets_r_base, offsets_l + start_l, offsets_r + start_r, num, num_l == num_r);
   num_l-= num, num_r-= num;
   start_l+= num, start_r+= num;
   if(num_l == 0) start_l= 0, offsets_l_base= first;
   if(num_r == 0) start_r= 0, offsets_r_base= last;
  }
  if(num_l) {
   offsets_l+= start_l;
   while(num_l--) std::iter_swap(offsets_l_base + offsets_l[num_l], --last);
   first= last;
  }
  if(num_r) {
   offsets_r+= start_r;
   while(num_r--) std::iter_swap(offsets_r_base - offsets_r[num_r], first), ++first;
   last= first;
  }
 }
 u32* pivot_pos= first - 1;
 *begin= *pivot_pos;
 *pivot_pos= pivot;
 return {pivot_pos, already_partitioned};
}
// 軸 *begin 以下の要素を左に、軸より大きい要素を右に分ける。軸と等しい要素が多いときに使う。
inline u32* partition_left(u32* begin, u32* end) {
 const u32 pivot= *begin;
 u32 *first= begin, *last= end;
 while(pivot < *--last);
 if(last + 1 == end)
  while(first < last && !(pivot < *++first));
 else
  while(!(pivot < *++first));
 while(first < last) {
  std::iter_swap(first, last);
  while(pivot < *--last);
  while(!(pivot < *++first));
 }
 u32* pivot_pos= last;
 *begin= *pivot_pos;
 *pivot_pos= pivot;
 return pivot_pos;
}
inline void loop(u32* begin, u32* end, int bad_allowed, bool leftmost) {
 for(;;) {
  const ptrdiff_t size= end - begin;
  if(size < INSERTION_SORT_THRESHOLD) {
   if(leftmost) insertion_sort(begin, end);
   else unguarded_insertion_sort(begin, end);
   return;
  }
  const ptrdiff_t s2= size / 2;
  if(size > NINTHER_THRESHOLD) {
   sort3(begin, begin + s2, end - 1);
   sort3(begin + 1, begin + (s2 - 1), end - 2);
   sort3(begin + 2, begin + (s2 + 1), end - 3);
   sort3(begin + (s2 - 1), begin + s2, begin + (s2 + 1));
   std::iter_swap(begin, begin + s2);
  } else sort3(begin + s2, begin, end - 1);
  // *(begin - 1) は直前の分割の右側の区間の境なので、[begin, end) にそれより小さい要素は無い。
  // 軸がそれと等しければ、等しい要素を左に寄せる分割にして、左側 (全部等しい) には再帰しない。
  if(!leftmost && !(*(begin - 1) < *begin)) {
   begin= partition_left(begin, end) + 1;
   continue;
  }
  auto [pivot_pos, already_partitioned]= partition_right_branchless(begin, end);
  const ptrdiff_t l_size= pivot_pos - begin, r_size= end - (pivot_pos + 1);
  const bool highly_unbalanced= l_size < size / 8 || r_size < size / 8;
  if(highly_unbalanced) {
   if(--bad_allowed == 0) {
    std::make_heap(begin, end);
    std::sort_heap(begin, end);
    return;
   }
   if(l_size >= INSERTION_SORT_THRESHOLD) {
    std::iter_swap(begin, begin + l_size / 4);
    std::iter_swap(pivot_pos - 1, pivot_pos - l_size / 4);
    if(l_size > NINTHER_THRESHOLD) {
     std::iter_swap(begin + 1, begin + (l_size / 4 + 1));
     std::iter_swap(begin + 2, begin + (l_size / 4 + 2));
     std::iter_swap(pivot_pos - 2, pivot_pos - (l_size / 4 + 1));
     std::iter_swap(pivot_pos - 3, pivot_pos - (l_size / 4 + 2));
    }
   }
   if(r_size >= INSERTION_SORT_THRESHOLD) {
    std::iter_swap(pivot_pos + 1, pivot_pos + (1 + r_size / 4));
    std::iter_swap(end - 1, end - r_size / 4);
    if(r_size > NINTHER_THRESHOLD) {
     std::iter_swap(pivot_pos + 2, pivot_pos + (2 + r_size / 4));
     std::iter_swap(pivot_pos + 3, pivot_pos + (3 + r_size / 4));
     std::iter_swap(end - 2, end - (1 + r_size / 4));
     std::iter_swap(end - 3, end - (2 + r_size / 4));
    }
   }
  } else if(already_partitioned && partial_insertion_sort(begin, pivot_pos) && partial_insertion_sort(pivot_pos + 1, end)) return;
  loop(begin, pivot_pos, bad_allowed, leftmost);
  begin= pivot_pos + 1;
  leftmost= false;
 }
}
inline void sort(std::vector<u32>& v) {
 if(v.empty()) return;
 int log= 0;
 for(size_t n= v.size(); n >>= 1;) ++log;
 loop(v.data(), v.data() + v.size(), log, true);
}
}  // namespace sort_pdq
inline void run(std::vector<unsigned>& a) { sort_pdq::sort(a); }
