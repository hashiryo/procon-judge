#pragma once
// 診断用。整列の指定の無い operator new と delete を差し替えて、2 MB 以上の確保が載るページを選ぶ。PAGES_HP なら 2 MB 境界の
// mmap に置いて MADV_HUGEPAGE を頼み、PAGES_4K なら mmap に置いて MADV_NOHUGEPAGE で 4 KB のページに留める。2 MB 未満は
// malloc に回す。Linux でなければ何もしない。atcoder-abc155-e の診断 (9c7b4e18、208682d0 で消した) で使った形を戻したもの。
// NeoLibrary の MaxFlow、MinCostFlow、PrimalDual の網の配列を 2 MB の区画に置くと速くなるかの上限を見るため、メモリの大きい
// 6 問 (cf-1630-f、luogu-P4001、luogu-P4177、self-flow-maxflow-shapes、self-flow-mincostflow、self-flow-mincostflow-slope) の
// lib-neo-hp が使う。CI の x64 のランナーは透過的な huge page が madvise なので、頼まない配列は 4 KB のページで、初めて触る
// たびに OS がページを用意し、TLB にも収まらない。入力を読む配列など、ライブラリの外の 2 MB 以上の確保も同じく載せ替わる。
// PAGES_HP_STAGGER は PAGES_HP と同じく huge page を頼み、k 回目の確保の先頭を (k mod 16) 倍の 2 KB + 64 byte だけずらす
// (_shared/pages/stagger_new.hpp と同じずらし方)。lib-neo-hp では、頂点が 100 万ある luogu-P4001 が Xeon 6975P-C で 2 割遅く
// なった。頂点ごとの配列がどれも 2 MB 境界から始まり、同じ添字の要素がキャッシュの同じ組を取り合う形と見て、lib-neo-hps で
// 確かめる。
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#ifdef __linux__
#include <sys/mman.h>
#endif
namespace pages_plain_internal {
constexpr std::size_t H= std::size_t(1) << 21, P= 4096;
struct Block {
 void *p, *base;  // 返した番地と、mmap した領域の先頭
 std::size_t bytes;
};
inline unsigned cnt= 0;
inline Block blocks[64];
inline int nb= 0;
inline void* map([[maybe_unused]] std::size_t n) {
#ifdef __linux__
 [[maybe_unused]] const std::size_t bytes= (n + P - 1) & ~(P - 1);
 if(nb == 64) return nullptr;
#if defined(PAGES_HP) || defined(PAGES_HP_STAGGER)
#if defined(PAGES_HP_STAGGER)
 const std::size_t off= (cnt++ % 16) * (2048 + 64), len= (n + off + P - 1) & ~(P - 1);
#else
 const std::size_t off= 0, len= bytes;
#endif
 void* m= mmap(nullptr, len + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 if(m == MAP_FAILED) return nullptr;
 char *p= static_cast<char*>(m), *a= reinterpret_cast<char*>((reinterpret_cast<std::uintptr_t>(p) + H - 1) & ~(H - 1));
 if(a != p) munmap(p, a - p);
 munmap(a + len, p + H - a);
 madvise(a, len, MADV_HUGEPAGE);
 blocks[nb++]= {a + off, a, len};
 return a + off;
#elif defined(PAGES_4K)
 void* m= mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 if(m == MAP_FAILED) return nullptr;
 char* a= static_cast<char*>(m);
 madvise(a, bytes, MADV_NOHUGEPAGE);
 blocks[nb++]= {a, a, bytes};
 return a;
#else
 return nullptr;
#endif
#else
 return nullptr;
#endif
}
inline bool unmap(void* p) {
 for(int i= 0; i < nb; ++i)
  if(blocks[i].p == p) {
#ifdef __linux__
   munmap(blocks[i].base, blocks[i].bytes);
#endif
   blocks[i]= blocks[--nb];
   return true;
  }
 return false;
}
// 差し替えた operator new は 2 MB 未満を malloc で取るので、free で返すのが正しい。g++ は operator new が差し替えられたことを
// 見ずに -Wmismatched-new-delete を出すので、ここだけ抑止する。
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
inline void release(void* p) {
 if(p && !unmap(p)) std::free(p);
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}  // namespace pages_plain_internal
// operator new と delete は展開させない。PAGES_HP_STAGGER を g++-15 で組むと、self-flow-maxflow-shapes で、MaxFlow::run に
// 展開された一時の vector を捨てる delete が台帳 (blocks と nb) に番地を見つけられず、mmap した番地を free に渡して落ちた
// (-fno-assume-sane-operators-new-delete を付けても落ち、noinline にすると落ちない)。libstdc++ 15 の std::allocator は
// __builtin_operator_new と __builtin_operator_delete で呼ぶので、GCC はその呼び出しが確保と解放のほかに見える状態を変え
// ないと扱い、展開された delete が new の前に読んだ台帳の値を使い回したと見ている。展開させなければ、台帳を読むのは
// operator new と delete の中だけになる。
[[gnu::noinline]] void* operator new(std::size_t n) {
 if(n >= pages_plain_internal::H)
  if(void* p= pages_plain_internal::map(n)) return p;
 if(void* p= std::malloc(n ? n : 1)) return p;
 throw std::bad_alloc();
}
[[gnu::noinline]] void* operator new[](std::size_t n) { return ::operator new(n); }
[[gnu::noinline]] void operator delete(void* p) noexcept { pages_plain_internal::release(p); }
[[gnu::noinline]] void operator delete(void* p, std::size_t) noexcept { pages_plain_internal::release(p); }
[[gnu::noinline]] void operator delete[](void* p) noexcept { pages_plain_internal::release(p); }
[[gnu::noinline]] void operator delete[](void* p, std::size_t) noexcept { pages_plain_internal::release(p); }
