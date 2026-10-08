#pragma once
// 挿入ソート。前から順に、それまでに並べた列へ後ろから比べて差し込む。比較は N^2 / 4 回ほどで、配列が短いうちは呼ぶたびの
// 手間がいちばん少ない。長くなると比較の回数と分岐の予測の外れが効いてくる。
#include <vector>
namespace small_insertion {
using u32= unsigned;
inline void sort(std::vector<u32>& v) {
 u32* a= v.data();
 const size_t n= v.size();
 for(size_t i= 1; i < n; ++i) {
  const u32 x= a[i];
  size_t j= i;
  for(; j > 0 && a[j - 1] > x; --j) a[j]= a[j - 1];
  a[j]= x;
 }
}
}  // namespace small_insertion
inline void run(std::vector<unsigned>& a) { small_insertion::sort(a); }
