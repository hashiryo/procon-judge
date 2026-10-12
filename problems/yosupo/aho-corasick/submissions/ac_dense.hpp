#pragma once
// 各頂点に 26 個の子の番号を持つ表でトライを作り、幅優先の順に suffix link を求める素直な形。文字列を順に入れて、新しい頂点に
// できた順の番号を振る (問題の番号と同じ)。子の表は 0 を「無し」とし (根は誰の子でもない)、確保したばかりの 0 の領域をそのまま
// 使う。suffix link は、頂点 v = (親 p の文字 c の子) について、p の suffix link から始めて c の子を持つ頂点まで suffix link を
// たどり、その子とする (深さ 1 なら根)。たどる回数は全体で文字の数の和以下。表は 10^6 頂点で 104 MB になるので、2 MB 境界に寄せた
// 領域に madvise(MADV_HUGEPAGE) を呼んで置く (Linux のみ)。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#ifdef __linux__
#include <sys/mman.h>
#endif
#include "pj.hpp"

namespace ac_dense {
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
    p = calloc(bytes, 1);
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
}  // namespace ac_dense

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    size_t total = 0;
    for (const auto &s : ss) total += s.size();
    const int cap = int(total) + 1;
    ac_dense::Zeroed tab(size_t(cap) * 26 * sizeof(int));
    int(*ch)[26] = static_cast<int(*)[26]>(tab.get());
    vector<int> &par = ans.par, &link = ans.link;
    par.assign(cap, 0), link.assign(cap, 0), ans.word.resize(ss.size());
    vector<unsigned char> last(cap, 0);
    int n = 1;
    for (size_t i = 0; i < ss.size(); ++i) {
      int v = 0;
      for (const char x : ss[i]) {
        const int c = x - 'a';
        if (!ch[v][c]) ch[v][c] = n, par[n] = v, last[n] = (unsigned char)c, ++n;
        v = ch[v][c];
      }
      ans.word[i] = v;
    }
    // 幅優先の順に suffix link を求める。
    vector<int> q(n);
    int head = 0, tail = 0;
    for (int c = 0; c < 26; ++c)
      if (ch[0][c]) q[tail++] = ch[0][c];
    while (head < tail) {
      const int v = q[head++];
      if (par[v] != 0) {
        const int c = last[v];
        int u = link[par[v]];
        while (u && !ch[u][c]) u = link[u];
        link[v] = ch[u][c] && ch[u][c] != v ? ch[u][c] : 0;
      }
      for (int c = 0; c < 26; ++c)
        if (ch[v][c]) q[tail++] = ch[v][c];
    }
    par.resize(n), link.resize(n);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
