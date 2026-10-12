#pragma once
// dq_tk で、頂点の列、direct link の表、各位置の表面 2 本、文字の列を、遷移の表と同じ 0 で埋まった領域 (2 MB 以上なら、Linux では
// 2 MB 境界の mmap に置いて huge page を頼む) に置き、答えの列は領域を取ってから huge page を頼む版。EVEN を 0 番にして、表面を
// EVEN で埋める手間を省く。1 回目 (df9071f1 の run 38214426197、EPYC 9V45) の x64-gcc では、dq_remap が 15.42 ms、dq_tk が
// 15.67 ms だった。dq_tk は 5 * 10^5 個のクエリで、0 で埋めた頂点の列 (12 MB)、EVEN で埋めた表面 2 本 (8 MB)、答え (6 MB) などを
// 4 KB のページに取っていて、確保したばかりの領域に初めて書く代価 (EPYC 7763 で 1 MB あたり 0.43 ms) で 10 ms を超える。
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

namespace dq_tk2 {
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
constexpr int EVEN = 0, ODD = 1;  // EVEN を 0 番にして、0 で埋まった領域の表面をそのまま EVEN にする (子の 0 は「無し」のまま)
// v に n 個分の領域を取り (中身はまだ書かない)、Linux ではその領域のページに huge page を頼む。vector の領域は 2 MB 境界から
// 始まるとは限らないので、huge page に載るのは領域の内側にある 2 MB の区切りだけになる。
template <class T> inline void reserve_huge(std::vector<T> &v, size_t n) {
  v.reserve(n);
#ifdef __linux__
  const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
  if (b < e) madvise(reinterpret_cast<void *>(b), e - b, MADV_HUGEPAGE);
#endif
}
struct Node {
  int len, link, par, cnt, linkcnt, ch;
};
}  // namespace dq_tk2

struct Solver {
  vector<pair<int, char>> qs;
  vector<array<int, 3>> ans;

  explicit Solver(const vector<pair<int, char>> &qs) : qs(qs) {}

  template <int K> void body(const array<int, 256> &code) {
    using namespace dq_tk2;
    const int k = K;
    const int q = int(qs.size()), cap = q + 2, off = q + 1;
    // 遷移と direct link の表、頂点の列、各位置の表面 2 本、文字の列を、0 で埋まった 1 つの領域に置く
    const size_t P = 2 * size_t(q) + 3;
    Zeroed buf(size_t(cap) * k * 2 * sizeof(int) + size_t(cap) * sizeof(Node) + 2 * P * sizeof(int) + P);
    int *nxt = static_cast<int *>(buf.get()), *dlt = nxt + size_t(cap) * k;
    auto nx = [&](int v) { return nxt + size_t(v) * k; };
    auto dl = [&](int v) { return dlt + size_t(v) * k; };
    Node *nd = reinterpret_cast<Node *>(dlt + size_t(cap) * k);
    nd[ODD] = {-1, ODD, ODD, 0, 0, 0}, nd[EVEN] = {0, ODD, ODD, 0, 0, 0};
    for (int c = 0; c < k; ++c) dl(ODD)[c] = dl(EVEN)[c] = ODD;
    int *pre = reinterpret_cast<int *>(nd + cap), *suf = pre + P;  // 各位置から始まる表面、各位置で終わる表面 (0 = EVEN で埋まっている)
    unsigned char *data = reinterpret_cast<unsigned char *>(suf + P);
    int start = off, end = off, m = 2, live = 0;
    reserve_huge(ans, q);
    ans.resize(q);
    // u から c で伸ばした回文の頂点を返す (無ければ作る)。ext(w) は、回文 w の出現の外側の文字が c か。pc(L) は、新しい頂点の中で
    // 長さ L の回文の接尾辞 (接頭辞) の外側に隣る文字。
    auto grow = [&](int u, int c, auto ext, auto pc) {
      int &e = nx(u)[c];
      if (e) return e;
      const int x = m++;
      int lx = EVEN;
      if (u != ODD) {
        const int lu = nd[u].link;
        const int w = lu == ODD || ext(lu) ? lu : dl(lu)[c];
        lx = nx(w)[c];
      }
      nd[x] = {nd[u].len + 2, lx, u, 0, 0, c};
      ++nd[lx].linkcnt;
      memcpy(dl(x), dl(lx), sizeof(int) * k);
      dl(x)[pc(nd[lx].len)] = lx;
      ++live;
      return e = x;
    };
    for (int t = 0; t < q; ++t) {
      const auto [type, ch] = qs[t];
      if (type == 1) {  // 末尾に足す
        const int c = code[(unsigned char)ch], e = end;
        const int v = start == end ? EVEN : suf[e - 1];
        auto ext = [&](int w) { const int p = e - 1 - nd[w].len; return p >= start && data[p] == c; };
        const int u = ext(v) ? v : dl(v)[c];
        const int x = grow(u, c, ext, [&](int L) { return L == 0 ? c : int(data[e - L]); });
        data[e] = (unsigned char)c, end = e + 1;
        pre[e] = EVEN, pre[e + 1 - nd[x].len] = x, suf[e] = x;
        const int lx = nd[x].link, lq = e - nd[x].len + nd[lx].len;
        if (nd[lx].len >= 1 && suf[lq] == lx) suf[lq] = EVEN;
        ++nd[x].cnt;
      } else if (type == 0) {  // 先頭に足す
        const int c = code[(unsigned char)ch], b = start - 1;
        const int v = start == end ? EVEN : pre[start];
        auto ext = [&](int w) { const int p = start + nd[w].len; return p < end && data[p] == c; };
        const int u = ext(v) ? v : dl(v)[c];
        const int x = grow(u, c, ext, [&](int L) { return L == 0 ? c : int(data[b + L]); });
        data[b] = (unsigned char)c, start = b;
        pre[b] = x, suf[b] = EVEN, suf[b + nd[x].len - 1] = x;
        const int lx = nd[x].link, lq = b + nd[x].len - nd[lx].len;
        if (nd[lx].len >= 1 && pre[lq] == lx) pre[lq] = EVEN;
        ++nd[x].cnt;
      } else if (type == 3) {  // 末尾を除く
        const int e = end - 1, v = suf[e];
        if (nd[v].linkcnt == 0 && nd[v].cnt == 1) nx(nd[v].par)[nd[v].ch] = 0, --nd[nd[v].link].linkcnt, --live;
        --nd[v].cnt;
        const int lv = nd[v].link, st = e - nd[v].len + 1;
        if (nd[lv].len >= 1 && nd[suf[st + nd[lv].len - 1]].len < nd[lv].len) suf[st + nd[lv].len - 1] = lv, pre[st] = lv;
        else pre[st] = EVEN;
        end = e;
      } else {  // 先頭を除く
        const int b = start, v = pre[b];
        if (nd[v].linkcnt == 0 && nd[v].cnt == 1) nx(nd[v].par)[nd[v].ch] = 0, --nd[nd[v].link].linkcnt, --live;
        --nd[v].cnt;
        const int lv = nd[v].link, en = b + nd[v].len - 1;
        if (nd[lv].len >= 1 && nd[pre[en - nd[lv].len + 1]].len < nd[lv].len) pre[en - nd[lv].len + 1] = lv, suf[en] = lv;
        else suf[en] = EVEN;
        start = b + 1;
      }
      ans[t] = start == end ? array<int, 3>{0, 0, 0} : array<int, 3>{live, nd[pre[start]].len, nd[suf[end - 1]].len};
    }
  }

  void run() {
    array<int, 256> code;
    code.fill(-1);
    int k = 0;
    for (const auto &[type, ch] : qs)
      if (type <= 1 && code[(unsigned char)ch] < 0) code[(unsigned char)ch] = k++;
    if (k <= 2) body<2>(code);
    else if (k <= 4) body<4>(code);
    else if (k <= 8) body<8>(code);
    else body<26>(code);
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
