#pragma once
#include <algorithm>
#include <bit>
#include <cstring>
#include <new>
#include <vector>
#include "pj.hpp"
#include "neo/algebra/ZMod.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#else
#include <simde/x86/avx2.h>
#endif
#ifdef __linux__
#include <sys/mman.h>
#endif

// _chunk2.hpp の木の部分を treap から AVL 木に替えたもの (chunk2_avl256.hpp が B を決めて使う)。塊の中の処理、反転の入れ替え、
// 作り直しは _chunk2.hpp と同じで、split と併合は avl.hpp と同じ join (高い方の背を下りて 1 回か 2 回の回転で直す) で書く。
// 空になった節を外したときは、根までの道の節を join でつなぎ直して高さの差を直す。以下は _chunk2.hpp の説明。
//  - 塊の中の 1 次関数の作用と和を AVX2 で 8 個ずつ行う。値はそのままの値を 2p 未満で持ち、掛け算は Shoup の方法
//    (NeoLibrary の segtree/Affine.hpp と同じ)。
//  - 区間の和は塊を書き換えない。塊の遅延が残っていれば部分の和に当てる。部分が塊の半分より長ければ、残りの部分の和を
//    塊の和から引く。
//  - 塊の一部への作用は、残っている塊の遅延と合わせて 1 回の走査で書く (区間の内と外で掛ける数をレーンごとに選ぶ)。
//  - 塊の反転の印は配列を並べ替えずに添字を読み替える。並べ替えるのは、塊を割るとき、反転で塊の中身を入れ替えるとき、
//    作り直しのときだけ。
//  - 塊の途中で割るときは、新しい節に乱数の優先度を与えて右の部分木と merge する。chunk.hpp は元の節の優先度を
//    引き継いでいて、位置 1 への挿入を続ける extreme_insertion_01 で同じ優先度の節が 1 本の道につながった
//    (作り直しまで、深さが節の数の半分まで伸びた)。
//  - 満杯の塊への挿入は、塊を半分ずつの 2 つの節に割ってから入れる。削除で空になった節は木から外す。
//  - 区間の反転は、両端の塊のうち区間の外にある部分を逆順にして互いの塊へ移してから、塊の境目で split して印を付ける
//    (Library Checker の nandhagk の 402977 の方法)。塊を割らないので節が増えない。移したあとの塊が B を超えるときだけ、
//    chunk.hpp と同じく塊を割る。
namespace chunk2avl_internal {
using u32 = unsigned;
using u64 = unsigned long long;
using Z = ZMod<998244353>;
constexpr u32 P = 998244353, P2 = 2 * P;
// x -> m x + c。m と c は p 未満で、q は Shoup の商の近似 floor(m 2^32 / p)。
struct Act {
  __m256i m, q, c;
};
inline Act act(Z b, Z c) {
  const u32 w = b.val();
  return {_mm256_set1_epi32(int(w)), _mm256_set1_epi32(int(u32((u64(w) << 32) / P))), _mm256_set1_epi32(int(c.val()))};
}
// x m mod p を 2p 未満で。x は u32 なら何でもよい。m と q はどのレーンも同じ値であること (奇数のレーンの積にも偶数の
// レーンの q を使う)。
[[gnu::always_inline]] inline __m256i mul(__m256i x, __m256i m, __m256i q) {
  const __m256i e = _mm256_mul_epu32(x, q), o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), q);
  const __m256i h = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, m), _mm256_mullo_epi32(h, _mm256_set1_epi32(int(P))));
}
// mul で、m と q がレーンごとに違ってよい形。
[[gnu::always_inline]] inline __m256i mul_lanes(__m256i x, __m256i m, __m256i q) {
  const __m256i e = _mm256_mul_epu32(x, q), o = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(q, 32));
  const __m256i h = _mm256_blend_epi32(_mm256_srli_epi64(e, 32), o, 0xaa);
  return _mm256_sub_epi32(_mm256_mullo_epi32(x, m), _mm256_mullo_epi32(h, _mm256_set1_epi32(int(P))));
}
// a + b を 2p 未満で。a, b < 2p。
[[gnu::always_inline]] inline __m256i add(__m256i a, __m256i b) {
  const __m256i s = _mm256_add_epi32(a, b);
  return _mm256_min_epu32(s, _mm256_sub_epi32(s, _mm256_set1_epi32(int(P2))));
}
[[gnu::always_inline]] inline __m256i fma(__m256i x, __m256i m, __m256i q, __m256i c) { return add(mul(x, m, q), c); }
inline u32 hsum(__m256i s) {
  const __m128i p2 = _mm_set1_epi32(int(P2));
  __m128i t = _mm_add_epi32(_mm256_castsi256_si128(s), _mm256_extracti128_si256(s, 1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0x4e));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  t = _mm_add_epi32(t, _mm_shuffle_epi32(t, 0xb1));
  t = _mm_min_epu32(t, _mm_sub_epi32(t, p2));
  return u32(_mm_cvtsi128_si32(t));
}
[[gnu::always_inline]] inline __m256i iota8() { return _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7); }
[[gnu::always_inline]] inline __m256i ld(const u32 *p) { return _mm256_load_si256((const __m256i *)p); }
[[gnu::always_inline]] inline void st(u32 *p, __m256i x) { _mm256_store_si256((__m256i *)p, x); }
// a[lo, hi) の和を 2p 未満で。
inline u32 sum_range(const u32 *a, int lo, int hi) {
  if (lo >= hi) return 0;
  const __m256i L = _mm256_set1_epi32(lo - 1), H = _mm256_set1_epi32(hi), step = _mm256_set1_epi32(8);
  int k = lo >> 3;
  const int ke = (hi + 7) >> 3;
  __m256i idx = _mm256_add_epi32(iota8(), _mm256_set1_epi32(k << 3)), s0 = _mm256_setzero_si256(), s1 = s0;
  for (; k + 1 < ke; k += 2) {
    const __m256i i1 = _mm256_add_epi32(idx, step);
    const __m256i k0 = _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
    const __m256i k1 = _mm256_and_si256(_mm256_cmpgt_epi32(i1, L), _mm256_cmpgt_epi32(H, i1));
    s0 = add(s0, _mm256_and_si256(ld(a + (k << 3)), k0));
    s1 = add(s1, _mm256_and_si256(ld(a + (k << 3) + 8), k1));
    idx = _mm256_add_epi32(i1, step);
  }
  if (k < ke) {
    const __m256i k0 = _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
    s0 = add(s0, _mm256_and_si256(ld(a + (k << 3)), k0));
  }
  return hsum(add(s0, s1));
}
// a[0, len) に f を当てる。
inline void transform_all(u32 *a, int len, const Act &f) {
  for (int k = 0; k < len; k += 8) st(a + k, fma(ld(a + k), f.m, f.q, f.c));
}
// a[lo, hi) に f を当て、当てる前の和を 2p 未満で返す。
inline u32 transform_range(u32 *a, int lo, int hi, const Act &f) {
  const __m256i L = _mm256_set1_epi32(lo - 1), H = _mm256_set1_epi32(hi), step = _mm256_set1_epi32(8);
  int k = lo >> 3;
  const int ke = (hi + 7) >> 3;
  __m256i idx = _mm256_add_epi32(iota8(), _mm256_set1_epi32(k << 3)), s = _mm256_setzero_si256();
  for (; k < ke; ++k) {
    const __m256i x = ld(a + (k << 3));
    const __m256i msk = _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
    st(a + (k << 3), _mm256_blendv_epi8(x, fma(x, f.m, f.q, f.c), msk));
    s = add(s, _mm256_and_si256(x, msk));
    idx = _mm256_add_epi32(idx, step);
  }
  return hsum(s);
}
// a[0, len) のうち [lo, hi) に in を、ほかに out を当て、当てたあとの和を 2p 未満で返す。
inline u32 transform2(u32 *a, int len, int lo, int hi, const Act &in, const Act &out) {
  const __m256i L = _mm256_set1_epi32(lo - 1), H = _mm256_set1_epi32(hi), N = _mm256_set1_epi32(len), step = _mm256_set1_epi32(8);
  __m256i idx = iota8(), s = _mm256_setzero_si256();
  for (int k = 0; k < len; k += 8) {
    const __m256i msk = _mm256_and_si256(_mm256_cmpgt_epi32(idx, L), _mm256_cmpgt_epi32(H, idx));
    const __m256i m = _mm256_blendv_epi8(out.m, in.m, msk), q = _mm256_blendv_epi8(out.q, in.q, msk), c = _mm256_blendv_epi8(out.c, in.c, msk);
    const __m256i y = add(mul_lanes(ld(a + k), m, q), c);
    st(a + k, y);
    s = add(s, _mm256_and_si256(y, _mm256_cmpgt_epi32(N, idx)));
    idx = _mm256_add_epi32(idx, step);
  }
  return hsum(s);
}
// 1 MB 以上の確保は 2 MB 境界に置き、Linux では huge page を頼む。
inline u32 *big_alloc(size_t bytes) {
  if (bytes < (size_t(1) << 20)) return static_cast<u32 *>(::operator new(bytes, std::align_val_t(64)));
  const size_t H = size_t(1) << 21, real = (bytes + H - 1) & ~(H - 1);
  void *p = ::operator new(real, std::align_val_t(H));
#ifdef __linux__
  madvise(p, real, MADV_HUGEPAGE);
#endif
  return static_cast<u32 *>(p);
}
inline void big_free(u32 *p, size_t bytes) {
  if (!p) return;
  if (bytes < (size_t(1) << 20)) ::operator delete(p, std::align_val_t(64));
  else ::operator delete(p, std::align_val_t(size_t(1) << 21));
}
}  // namespace chunk2avl_internal

template <int B> struct ChunkSolver2Avl {
  static_assert(B % 16 == 0);
  using Z = chunk2avl_internal::Z;
  using u32 = chunk2avl_internal::u32;
  struct Node {
    int l, r, cnt, len, h;
    Z sum, csum, b, c, cb, cc;  // (b, c) は子へ配る分、(cb, cc) は自分の配列へ当てる分。sum と csum は両方を当てたあとの値
    bool rev, lz, crev, clz;
  };
  std::vector<Node> t;
  u32 *dat = nullptr;  // 節 x の配列は dat[x B, x B + len)
  size_t dcap = 0;     // dat に置ける節の数
  int root = 0, limit = 0;
  std::vector<int> path, path2;
  u32 *arr(int x) { return dat + size_t(x) * B; }
  static u32 raw(Z z) { return std::bit_cast<u32>(z); }
  void reserve_chunks(size_t need) {
    if (need <= dcap) return;
    const size_t nc = std::max(need, dcap * 2), bytes = nc * B * sizeof(u32);
    u32 *q = chunk2avl_internal::big_alloc(bytes);
    if (dat) std::memcpy(q, dat, dcap * B * sizeof(u32));
    std::memset(q + dcap * B, 0, (nc - dcap) * B * sizeof(u32));
    chunk2avl_internal::big_free(dat, dcap * B * sizeof(u32));
    dat = q, dcap = nc;
  }
  int make() {
    t.push_back({0, 0, 0, 0, 1, Z(), Z(), Z(1), Z(), Z(1), Z(), false, false, false, false});
    reserve_chunks(t.size());
    return (int)t.size() - 1;
  }
  void aff(int x, Z b, Z c) {
    Node &n = t[x];
    n.sum = b * n.sum + c * Z(n.cnt), n.csum = b * n.csum + c * Z(n.len);
    n.b = b * n.b, n.c = b * n.c + c, n.lz = true;
    n.cb = b * n.cb, n.cc = b * n.cc + c, n.clz = true;
  }
  void tog(int x) { std::swap(t[x].l, t[x].r), t[x].rev ^= 1, t[x].crev ^= 1; }
  void push(int x) {
    Node &n = t[x];
    if (n.rev) {
      if (n.l) tog(n.l);
      if (n.r) tog(n.r);
      n.rev = false;
    }
    if (n.lz) {
      if (n.l) aff(n.l, n.b, n.c);
      if (n.r) aff(n.r, n.b, n.c);
      n.b = Z(1), n.c = Z(), n.lz = false;
    }
  }
  void pull(int x) {
    Node &n = t[x];
    n.cnt = t[n.l].cnt + n.len + t[n.r].cnt, n.sum = t[n.l].sum + n.csum + t[n.r].sum;
    n.h = 1 + std::max(t[n.l].h, t[n.r].h);
  }
  // 塊の遅延の作用を配列に当てる (反転の印は残す)。
  void materialize(int x) {
    Node &n = t[x];
    if (!n.clz) return;
    chunk2avl_internal::transform_all(arr(x), n.len, chunk2avl_internal::act(n.cb, n.cc));
    n.cb = Z(1), n.cc = Z(), n.clz = false;
  }
  // 作用を当てて、配列を列の順に並べる。
  void forward(int x) {
    materialize(x);
    if (t[x].crev) std::reverse(arr(x), arr(x) + t[x].len), t[x].crev = false;
  }
  // 塊の中の列の順の [cl, cr) の和。
  Z part_sum(int x, int cl, int cr) {
    const Node &n = t[x];
    const int len = n.len, pl = n.crev ? len - cr : cl, pr = n.crev ? len - cl : cr, w = pr - pl;
    const u32 *a = arr(x);
    if (2 * w <= len) {
      const Z s = Z::raw(chunk2avl_internal::sum_range(a, pl, pr));
      return n.clz ? n.cb * s + n.cc * Z(w) : s;
    }
    const Z s = Z::raw(chunk2avl_internal::sum_range(a, 0, pl)) + Z::raw(chunk2avl_internal::sum_range(a, pr, len));
    return n.csum - (n.clz ? n.cb * s + n.cc * Z(len - w) : s);
  }
  // 塊の中の列の順の [cl, cr) (塊の全体ではない) に x -> b x + c を当てる。
  void part_apply(int x, int cl, int cr, Z b, Z c) {
    Node &n = t[x];
    const int len = n.len, pl = n.crev ? len - cr : cl, pr = n.crev ? len - cl : cr;
    if (n.clz) {
      const auto in = chunk2avl_internal::act(b * n.cb, b * n.cc + c), out = chunk2avl_internal::act(n.cb, n.cc);
      n.csum = Z::raw(chunk2avl_internal::transform2(arr(x), len, pl, pr, in, out));
      n.cb = Z(1), n.cc = Z(), n.clz = false;
    } else {
      const Z old = Z::raw(chunk2avl_internal::transform_range(arr(x), pl, pr, chunk2avl_internal::act(b, c)));
      n.csum += (b - Z(1)) * old + c * Z(pr - pl);
    }
  }
  int h(int x) const { return t[x].h; }
  // 節 k (遅延を配ってあること) を真ん中にして、子を l と r にする。
  int node(int l, int k, int r) {
    t[k].l = l, t[k].r = r, pull(k);
    return k;
  }
  int rot_left(int x) {
    const int y = t[x].r;
    push(y);
    t[x].r = t[y].l, pull(x);
    t[y].l = x, pull(y);
    return y;
  }
  int rot_right(int x) {
    const int y = t[x].l;
    push(y);
    t[x].l = t[y].r, pull(x);
    t[y].r = x, pull(y);
    return y;
  }
  // l が r より 2 以上高いとき、l の右の背を下りて k と r をつなぐ。
  int join_right(int l, int k, int r) {
    push(l);
    const int a = t[l].l, c = t[l].r;
    if (h(c) <= h(r) + 1) {
      int x = node(c, k, r);
      if (h(x) <= h(a) + 1) return node(a, l, x);
      x = rot_right(x);
      return rot_left(node(a, l, x));
    }
    const int x = join_right(c, k, r);
    const int y = node(a, l, x);
    return h(x) <= h(a) + 1 ? y : rot_left(y);
  }
  int join_left(int l, int k, int r) {
    push(r);
    const int c = t[r].l, b = t[r].r;
    if (h(c) <= h(l) + 1) {
      int x = node(l, k, c);
      if (h(x) <= h(b) + 1) return node(x, r, b);
      x = rot_left(x);
      return rot_right(node(x, r, b));
    }
    const int x = join_left(l, k, c);
    const int y = node(x, r, b);
    return h(x) <= h(b) + 1 ? y : rot_right(y);
  }
  int join(int l, int k, int r) {
    if (h(l) > h(r) + 1) return join_right(l, k, r);
    if (h(r) > h(l) + 1) return join_left(l, k, r);
    return node(l, k, r);
  }
  // 先頭の k 個を a に、残りを b に分ける。k が塊の中に落ちたら、塊を 2 つの節に割る。
  void split(int x, int k, int &a, int &b) {
    if (!x) return void(a = b = 0);
    push(x);
    const int l = t[x].l, r = t[x].r, ls = t[l].cnt, len = t[x].len;
    if (k <= ls) {
      int p, q;
      split(l, k, p, q);
      a = p, b = join(q, x, r);
    } else if (k >= ls + len) {
      int p, q;
      split(r, k - ls - len, p, q);
      a = join(l, x, p), b = q;
    } else {
      const int o = k - ls;
      forward(x);
      const int y = make();
      u32 *ax = arr(x), *ay = arr(y);
      std::memcpy(ay, ax + o, size_t(len - o) * sizeof(u32));
      t[y].len = len - o, t[x].len = o;
      if (o <= len - o) {
        const Z s = Z::raw(chunk2avl_internal::sum_range(ax, 0, o));
        t[y].csum = t[x].csum - s, t[x].csum = s;
      } else {
        const Z s = Z::raw(chunk2avl_internal::sum_range(ay, 0, len - o));
        t[x].csum -= s, t[y].csum = s;
      }
      a = join(l, x, 0), b = join(0, y, r);
    }
  }
  // 最後の節を外して返し、残りを rest に入れる。
  int split_last(int x, int &rest) {
    push(x);
    if (!t[x].r) return rest = t[x].l, x;
    int r;
    const int m = split_last(t[x].r, r);
    rest = join(t[x].l, x, r);
    return m;
  }
  int merge(int a, int b) {
    if (!a || !b) return a | b;
    int rest;
    const int m = split_last(a, rest);
    return join(rest, m, b);
  }
  Z prod(int x, int l, int r) {
    if (!x || l >= r) return Z();
    if (l == 0 && r == t[x].cnt) return t[x].sum;
    push(x);
    const int ls = t[t[x].l].cnt, len = t[x].len;
    Z s;
    if (l < ls) s += prod(t[x].l, l, std::min(r, ls));
    const int cl = std::max(l - ls, 0), cr = std::min(r - ls, len);
    if (cl < cr) s += cl == 0 && cr == len ? t[x].csum : part_sum(x, cl, cr);
    if (r > ls + len) s += prod(t[x].r, std::max(l - ls - len, 0), r - ls - len);
    return s;
  }
  void apply(int x, int l, int r, Z b, Z c) {
    if (!x || l >= r) return;
    if (l == 0 && r == t[x].cnt) return aff(x, b, c);
    push(x);
    const int ls = t[t[x].l].cnt, len = t[x].len;
    if (l < ls) apply(t[x].l, l, std::min(r, ls), b, c);
    const int cl = std::max(l - ls, 0), cr = std::min(r - ls, len);
    if (cl < cr) {
      if (cl == 0 && cr == len) {
        Node &n = t[x];
        n.csum = b * n.csum + c * Z(len), n.cb = b * n.cb, n.cc = b * n.cc + c, n.clz = true;
      } else part_apply(x, cl, cr, b, c);
    }
    if (r > ls + len) apply(t[x].r, std::max(l - ls - len, 0), r - ls - len, b, c);
    pull(x);
  }
  // 位置 i を含む塊まで、遅延を配りながら下りる。end が真なら塊の後ろの端も含める。o に塊の中の位置を入れる。
  int descend(int i, bool end, std::vector<int> &ps, int &o) {
    ps.clear();
    int x = root;
    o = i;
    for (;;) {
      push(x), ps.push_back(x);
      const int ls = t[t[x].l].cnt;
      if (o < ls) x = t[x].l;
      else if (o < ls + t[x].len + (end ? 1 : 0)) {
        o -= ls;
        return x;
      } else o -= ls + t[x].len, x = t[x].r;
    }
  }
  void pull_path(const std::vector<int> &ps) {
    for (int k = (int)ps.size(); k--;) pull(ps[k]);
  }
  // 塊の中の列の順の位置 o に値 v を入れる (空きがあること)。
  void put(int x, int o, u32 v) {
    materialize(x);
    Node &n = t[x];
    u32 *a = arr(x);
    const int p = n.crev ? n.len - o : o;
    std::memmove(a + p + 1, a + p, size_t(n.len - p) * sizeof(u32));
    a[p] = v, ++n.len, n.csum += Z::raw(v);
  }
  // 値を並べ直して、B / 2 個ずつの塊で釣り合った木を作り直す。
  void collect(int x, std::vector<u32> &out) {
    if (!x) return;
    push(x), forward(x);
    collect(t[x].l, out);
    out.insert(out.end(), arr(x), arr(x) + t[x].len);
    collect(t[x].r, out);
  }
  int build(const std::vector<u32> &v, int lo, int hi) {  // 塊の番号の [lo, hi)
    if (lo >= hi) return 0;
    const int m = (lo + hi) / 2;
    const int x = make();
    const int s = m * (B / 2), e = std::min<int>(s + B / 2, (int)v.size());
    std::copy(v.begin() + s, v.begin() + e, arr(x));
    t[x].len = e - s, t[x].csum = Z::raw(chunk2avl_internal::sum_range(arr(x), 0, e - s));
    const int l = build(v, lo, m), r = build(v, m + 1, hi);
    t[x].l = l, t[x].r = r, pull(x);
    return x;
  }
  void rebuild(const std::vector<u32> &v) {
    t.resize(1), root = 0;
    const int chunks = ((int)v.size() + B / 2 - 1) / (B / 2);
    reserve_chunks(size_t(chunks) + 1);
    root = build(v, 0, chunks);
    limit = 2 * (int)t.size() + 1024;
  }
  void maybe_rebuild() {
    if ((int)t.size() <= limit) return;
    std::vector<u32> v;
    v.reserve(t[root].cnt);
    collect(root, v);
    rebuild(v);
  }
  explicit ChunkSolver2Avl(const std::vector<i64> &a) {
    t.reserve(1 << 14);
    t.push_back({0, 0, 0, 0, 0, Z(), Z(), Z(1), Z(), Z(1), Z(), false, false, false, false});  // 0 は空の木 (高さ 0)
    std::vector<u32> v(a.size());
    for (size_t i = 0; i < a.size(); ++i) v[i] = raw(Z(a[i]));
    rebuild(v);
  }
  ~ChunkSolver2Avl() { chunk2avl_internal::big_free(dat, dcap * B * sizeof(u32)); }
  ChunkSolver2Avl(const ChunkSolver2Avl &) = delete;
  ChunkSolver2Avl &operator=(const ChunkSolver2Avl &) = delete;
  void insert(int i, i64 x) {
    const u32 v = raw(Z(x));
    if (!root) {
      root = make();
      arr(root)[0] = v, t[root].len = 1, t[root].csum = Z::raw(v), pull(root);
      return;
    }
    int o;
    const int x0 = descend(i, true, path, o);
    if (t[x0].len < B) {
      put(x0, o, v);
      pull_path(path);
      return;
    }
    // 満杯なら、前半を x0 に残して後半を新しい節 y に移し、y を真ん中にして x0 のすぐ後ろへ join でつなぐ。
    forward(x0);
    const int y = make();
    u32 *ax = arr(x0), *ay = arr(y);
    std::memcpy(ay, ax + B / 2, size_t(B / 2) * sizeof(u32));
    t[y].len = B / 2, t[x0].len = B / 2;
    const Z sy = Z::raw(chunk2avl_internal::sum_range(ay, 0, B / 2));
    t[y].csum = sy, t[x0].csum -= sy;
    if (o <= B / 2) put(x0, o, v);
    else put(y, o - B / 2, v);
    pull_path(path);
    const int pos = i - o + t[x0].len;
    int a, b;
    split(root, pos, a, b);
    root = join(a, y, b);
    maybe_rebuild();
  }
  void erase(int i) {
    int o;
    const int x = descend(i, false, path, o);
    {
      Node &n = t[x];
      u32 *a = arr(x);
      const int p = n.crev ? n.len - 1 - o : o;
      const Z old = Z::raw(a[p]);
      n.csum -= n.clz ? n.cb * old + n.cc : old;
      std::memmove(a + p, a + p + 1, size_t(n.len - 1 - p) * sizeof(u32));
      --n.len;
    }
    if (!t[x].len) {  // 空になった節を外し、根までの道の節を join でつなぎ直す (x の遅延は descend で配ってある)
      int old = x, cur = merge(t[x].l, t[x].r);
      path.pop_back();
      for (int k = (int)path.size(); k--;) {
        const int p = path[k];
        (t[p].l == old ? t[p].l : t[p].r) = cur;
        old = p, cur = join(t[p].l, p, t[p].r);
      }
      root = cur;
      return;
    }
    pull_path(path);
  }
  void reverse(int l, int r) {
    if (r - l < 2) return;
    int i, jj;
    const int u = descend(l, false, path, i);
    const int v = descend(r - 1, false, path2, jj);
    const int j = jj + 1;
    if (u == v) {  // 塊の中だけの反転。和は変わらない
      const Node &n = t[u];
      const int pl = n.crev ? n.len - j : i, pr = n.crev ? n.len - i : j;
      std::reverse(arr(u) + pl, arr(u) + pr);
      return;
    }
    const int nu = t[u].len, nv = t[v].len, tail = nv - j;
    if (tail + nu - i > B || j + i > B) {
      int a, b, c;
      split(root, l, a, b);
      split(b, r - l, b, c);
      tog(b);
      root = merge(merge(a, b), c);
      maybe_rebuild();
      return;
    }
    if (i || tail) {
      // u の [0, i) と v の [j, nv) を逆順にして入れ替える: u = rev(v[j, nv)) + u[i, nu)、v = v[0, j) + rev(u[0, i))。
      forward(u), forward(v);
      u32 *U = arr(u), *V = arr(v);
      alignas(32) u32 tu[B], tv[B];
      std::memcpy(tu, U, size_t(i) * sizeof(u32));
      std::memcpy(tv, V + j, size_t(tail) * sizeof(u32));
      const Z su = Z::raw(chunk2avl_internal::sum_range(U, 0, i)), sv = Z::raw(chunk2avl_internal::sum_range(V, j, nv));
      std::memmove(U + tail, U + i, size_t(nu - i) * sizeof(u32));
      for (int k = 0; k < tail; ++k) U[k] = tv[tail - 1 - k];
      for (int k = 0; k < i; ++k) V[j + k] = tu[i - 1 - k];
      t[u].len = tail + nu - i, t[v].len = j + i;
      t[u].csum = t[u].csum - su + sv, t[v].csum = t[v].csum - sv + su;
      pull_path(path2), pull_path(path);
    }
    // 塊の境目 (u の先頭と v の末尾) で割って、間の木に反転の印を付ける。
    int a, b, c;
    split(root, l - i, a, b);
    split(b, r + tail - (l - i), b, c);
    tog(b);
    root = merge(merge(a, b), c);
  }
  void affine(int l, int r, i64 b, i64 c) { apply(root, l, r, Z(b), Z(c)); }
  i64 sum(int l, int r) { return prod(root, l, r).val(); }
};
