#pragma once
// 両端に文字を足し引きできる回文木 (Wang、Yang、Zhu の double-ended eertree、arXiv 2210.02292 の表面を記録する方法) を、26 文字の表で
// 素直に書いた形。回文の頂点ごとに、長さ、suffix link、親 (両端を除いた回文)、末尾の文字、cnt (最長の回文接尾辞として現れる位置の数、
// 最長の回文接頭辞として数えても同じ)、suffix link の木での子の数、文字ごとの遷移 (c + 自分 + c) と direct link (自分の真の回文接尾辞
// のうち、自分の中で直前が c である最長のもの。Rubinchik と Shur) を持つ。文字列の各位置には、そこから始まる表面と、そこで終わる
// 表面を持つ (表面は、ほかの回文の出現の接頭辞にも接尾辞にもならない回文の出現。最長の回文接頭辞は先頭の位置から始まる表面で、
// 最長の回文接尾辞は末尾の位置で終わる表面)。足すときは、最長の回文接尾辞 (接頭辞) から direct link を 1 回引いて伸ばせる回文を
// 見つけ、無ければ頂点を作る。新しい頂点の direct link は、suffix link の先の表を写して 1 か所を直す。除くときは、その回文が一意
// (子が無く cnt が 1) なら頂点を消す。どの操作にも繰り返しが無い。遷移の表は 0 を「無し」とし、2 MB 以上なら madvise(MADV_HUGEPAGE)
// を呼んだ領域に置く。
// 両端の回文木の実装を比べた記録は algo-notes の notes/palindromes-in-deque.md にある。
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace dq_basic {
// bytes byte の 0 で埋まった領域。2 MB 以上なら、Linux では 2 MB 境界の mmap に置いて huge page を頼む。
class Zeroed {
  void *p = nullptr;
#ifdef __linux__
  size_t mapped = 0;
#endif

 public:
  explicit Zeroed(size_t bytes) {
#ifdef __linux__
    constexpr size_t H = size_t(1) << 21;
    if (bytes >= H) {
      const size_t b = (bytes + 4095) & ~size_t(4095);
      void *m = mmap(nullptr, b + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (m != MAP_FAILED) {
        char *q = static_cast<char *>(m), *a = reinterpret_cast<char *>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
        if (a != q) munmap(q, a - q);
        munmap(a + b, q + H - a);
        madvise(a, b, MADV_HUGEPAGE);
        p = a, mapped = b;
        return;
      }
    }
#endif
    p = calloc(bytes ? bytes : 1, 1);
  }
  ~Zeroed() {
#ifdef __linux__
    if (mapped) {
      munmap(p, mapped);
      return;
    }
#endif
    free(p);
  }
  Zeroed(const Zeroed &) = delete;
  Zeroed &operator=(const Zeroed &) = delete;
  void *get() const { return p; }
};
constexpr int K = 26, ODD = 0, EVEN = 1;
struct Node {
  int len, link, par, cnt, linkcnt, ch;
};
}  // namespace dq_basic

struct Solver {
  vector<pair<int, char>> qs;
  vector<array<int, 3>> ans;

  explicit Solver(const vector<pair<int, char>> &qs) : qs(qs) {}

  void run() {
    using namespace dq_basic;
    const int q = int(qs.size()), cap = q + 2, off = q + 1;
    Zeroed tab(size_t(cap) * K * sizeof(int));
    int(*nx)[K] = static_cast<int(*)[K]>(tab.get());
    unique_ptr<int[][K]> dl(new int[cap][K]);
    vector<Node> nd(cap);
    nd[ODD] = {-1, ODD, ODD, 0, 0, 0}, nd[EVEN] = {0, ODD, ODD, 0, 0, 0};
    for (int c = 0; c < K; ++c) dl[ODD][c] = dl[EVEN][c] = ODD;
    vector<unsigned char> data(2 * size_t(q) + 3);
    vector<int> pre(2 * size_t(q) + 3, EVEN), suf(2 * size_t(q) + 3, EVEN);  // 各位置から始まる表面、各位置で終わる表面
    int start = off, end = off, m = 2, live = 0;
    ans.resize(q);
    // u から c で伸ばした回文の頂点を返す (無ければ作る)。ext(w) は、回文 w の出現の外側の文字が c か。pc(L) は、新しい頂点の中で
    // 長さ L の回文の接尾辞 (接頭辞) の外側に隣る文字。
    auto grow = [&](int u, int c, auto ext, auto pc) {
      int &e = nx[u][c];
      if (e) return e;
      const int x = m++;
      int lx = EVEN;
      if (u != ODD) {
        const int lu = nd[u].link;
        const int w = lu == ODD || ext(lu) ? lu : dl[lu][c];
        lx = nx[w][c];
      }
      nd[x] = {nd[u].len + 2, lx, u, 0, 0, c};
      ++nd[lx].linkcnt;
      memcpy(dl[x], dl[lx], sizeof(dl[x]));
      dl[x][pc(nd[lx].len)] = lx;
      ++live;
      return e = x;
    };
    for (int t = 0; t < q; ++t) {
      const auto [type, ch] = qs[t];
      if (type == 1) {  // 末尾に足す
        const int c = ch - 'a', e = end;
        const int v = start == end ? EVEN : suf[e - 1];
        auto ext = [&](int w) { const int p = e - 1 - nd[w].len; return p >= start && data[p] == c; };
        const int u = ext(v) ? v : dl[v][c];
        const int x = grow(u, c, ext, [&](int L) { return L == 0 ? c : int(data[e - L]); });
        data[e] = (unsigned char)c, end = e + 1;
        pre[e] = EVEN, pre[e + 1 - nd[x].len] = x, suf[e] = x;
        const int lx = nd[x].link, lq = e - nd[x].len + nd[lx].len;
        if (nd[lx].len >= 1 && suf[lq] == lx) suf[lq] = EVEN;
        ++nd[x].cnt;
      } else if (type == 0) {  // 先頭に足す
        const int c = ch - 'a', b = start - 1;
        const int v = start == end ? EVEN : pre[start];
        auto ext = [&](int w) { const int p = start + nd[w].len; return p < end && data[p] == c; };
        const int u = ext(v) ? v : dl[v][c];
        const int x = grow(u, c, ext, [&](int L) { return L == 0 ? c : int(data[b + L]); });
        data[b] = (unsigned char)c, start = b;
        pre[b] = x, suf[b] = EVEN, suf[b + nd[x].len - 1] = x;
        const int lx = nd[x].link, lq = b + nd[x].len - nd[lx].len;
        if (nd[lx].len >= 1 && pre[lq] == lx) pre[lq] = EVEN;
        ++nd[x].cnt;
      } else if (type == 3) {  // 末尾を除く
        const int e = end - 1, v = suf[e];
        if (nd[v].linkcnt == 0 && nd[v].cnt == 1) nx[nd[v].par][nd[v].ch] = 0, --nd[nd[v].link].linkcnt, --live;
        --nd[v].cnt;
        const int lv = nd[v].link, st = e - nd[v].len + 1;
        if (nd[lv].len >= 1 && nd[suf[st + nd[lv].len - 1]].len < nd[lv].len) suf[st + nd[lv].len - 1] = lv, pre[st] = lv;
        else pre[st] = EVEN;
        end = e;
      } else {  // 先頭を除く
        const int b = start, v = pre[b];
        if (nd[v].linkcnt == 0 && nd[v].cnt == 1) nx[nd[v].par][nd[v].ch] = 0, --nd[nd[v].link].linkcnt, --live;
        --nd[v].cnt;
        const int lv = nd[v].link, en = b + nd[v].len - 1;
        if (nd[lv].len >= 1 && nd[pre[en - nd[lv].len + 1]].len < nd[lv].len) pre[en - nd[lv].len + 1] = lv, suf[en] = lv;
        else suf[en] = EVEN;
        start = b + 1;
      }
      ans[t] = start == end ? array<int, 3>{0, 0, 0} : array<int, 3>{live, nd[pre[start]].len, nd[suf[end - 1]].len};
    }
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
