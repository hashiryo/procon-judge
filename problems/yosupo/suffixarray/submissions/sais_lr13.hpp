#pragma once
// sais_lr12 の induce_scan だけを sais_lr11 と同じ形 (2 つの形のループを 2 組ずつ持つ) に戻した版。7763 の回では、sais_lr12 は
// x64-clang でだけ sais_lr11 より 3 % から 10 % 遅く、all_same (最後の induce だけを通る) でも遅かったので、induce_scan のまとめ方を疑う。
// ほかは sais_lr12 と同じ。以下は sais_lr12 の説明。
// sais_lr11 を、計算はそのままに小さく書き直した版。induce_scan の 2 つの形 (バケットの位置をレジスタに置くか、置くたびに表を読み書き
// するか) を 1 組のループにまとめ、sort_lms_lr の前から読む走査と後ろから読む走査を 1 つのテンプレート (scan_lr) にまとめ、induce を
// 文字の種類が多い段の道に埋め込んだ。以下は、この版の組み立て。
// - 1 段目の文字は文字列の byte のまま読み、末尾には、どの文字より小さい番兵があるものとして扱う。L 型と S 型は配列に残さず、
//   隣の 2 文字から決める。LMS の位置は位置ごとの 1 bit の印で持ち、要るときに位置の列に広げる。
// - 文字の種類が少ない段 (sort_lms_lr) では、LMS の部分文字列を libsais の組み立て方で並べる。位置を「自分が L 型か」と「1 つ前が
//   L 型か」の 4 種類に分けて文字ごとに数え、前から読む走査で読む要素 (LMS と、1 つ前も L 型の L 型) を左の区画に、後ろから読む
//   走査で読む要素 (1 つ前が S 型の L 型と、1 つ前も S 型の S 型) を右の区画に、文字ごとに隙間なく置く。置き先を「置く要素の文字と、
//   その 1 つ前の種類」の 2 本のポインタで決めるので、どちらの走査も区画の要素を順にすべて読み、空きの判定も induce するかの判定も
//   要らない。いちばん左の LMS とそれより左の位置は数から外して種にもしない。LMS の部分文字列の順位は同じ走査で符号の印を付けて決め、
//   種にはバケットごとにいちばん前のものに印を付ける (付けないと、違う部分文字列に同じ名前が付いて再帰が 1 段増えることがある)。
// - 文字の種類が文字列の長さに比べて多い段 (K > n / 32、sort_lms_wide) では、文字ごとに 4 本ずつの表が x64 の L2 に収まらないので、
//   表 1 本の induced sorting で並べ、長さと文字の並びで比べて名前を付ける。
// - 最後の induced sorting は、置く値の符号に induce するかを持たせる Yuta Mori の sais-lite の手で、バケットの位置をレジスタに置く
//   形と、置くたびに表を読み書きする形 (libsais と同じ形) を、並んだ LMS の隣どうしで 1 つ前の文字が違う割合で選ぶ。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include "pj.hpp"

namespace sais_lr13 {
constexpr int MARK = int(0x80000000u), POS = 0x7fffffff;
// バケットの末尾に LMS を置いた sa を、L 型、S 型の順に induced sorting する。cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。
// sa に置く値の符号に「この接尾辞の 1 つ前を、今の走査で induce するか」を持たせる。L 型を前から induce する走査で置く j は L 型なので、
// j - 1 が S 型 (s[j - 1] < s[j]) なら ~j を置く。走査で読んだ値は反転して、L 型の走査で induce しなかったもの (前が S 型) を、S 型を
// 後ろから induce する走査で正の値として読む。空きは 0 にする (位置 0 の接尾辞と同じく、そこから induce しない)。
// First (1 回目、LMS の部分文字列を並べる) では読んだ値を 0 に戻し、S 型の走査で前が L 型の j (LMS) は ~j で置いてそのまま残すので、
// 走査が終わると LMS だけが負の値で、LMS の部分文字列の順に残る。最後の induced sorting では、負の値はすべて反転されて接尾辞の位置に
// 戻る。Table でなければ今書いているバケットの位置をレジスタに置き、文字が変わったときだけ表に書き戻す。Table (最後の induced sorting
// だけ) では置くたびに表を読み書きする。置くたびにバケットが替わる入力 (一様な文字列) では前者が書き戻しと分岐の分だけ遅く、
// 替わることが少ない入力 (同じバケットに続けて置く並びが長い) では後者がその待ちを積み重ねる。
template <bool First, bool Table, class C>
inline void induce_scan(const C *s, int n, int K, const int *cnt, int *sa, int *bkt) {
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c];
  if constexpr (Table) {
    {
      const int j = n - 1, c = int(s[j]);  // 番兵から最後の文字の接尾辞を induce する
      sa[bkt[c]++] = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c));
    }
    for (int i = 0; i < n; ++i) {
      int j = sa[i];
      sa[i] = ~j;
      if (j > 0) {
        --j;
        const int c = int(s[j]);
        sa[bkt[c]++] = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c));
      }
    }
    for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
    for (int i = n - 1; i >= 0; --i) {
      int j = sa[i];
      if (j > 0) {
        --j;
        const int c = int(s[j]);
        sa[--bkt[c]] = j ^ -int((j == 0) | (int(s[j - (j > 0)]) > c));
      } else {
        sa[i] = ~j;
      }
    }
    return;
  }
  int c1 = int(s[n - 1]);
  int *b = sa + bkt[c1];
  {
    const int j = n - 1;  // 番兵から最後の文字の接尾辞を induce する
    *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));
  }
  for (int i = 0; i < n; ++i) {
    int j = sa[i];
    sa[i] = First ? ~j & (j >> 31) : ~j;
    if (j > 0) {
      --j;
      const int c0 = int(s[j]);
      if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
      *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));  // ~j は j ^ -1。j = 0 のときは s[0] を読んで捨てる
    }
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  c1 = 0;
  b = sa + bkt[0];
  for (int i = n - 1; i >= 0; --i) {
    int j = sa[i];
    if (j > 0) {
      if constexpr (First) sa[i] = 0;
      --j;
      const int c0 = int(s[j]);
      if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
      const int lt = int(s[j - (j > 0)]) > c1;  // j - 1 が L 型
      *--b = j ^ -(First ? int(j > 0) & lt : int(j == 0) | lt);
    } else if constexpr (!First) {
      sa[i] = ~j;
    }
  }
}
// sa[0, m) に並んだ LMS の接尾辞を、その場でバケットの末尾へ移してから、最後の induced sorting をする。k 番目に小さい LMS の行き先は
// k 以上なので、後ろから移せば、まだ移していないものを上書きしない。
template <bool Table, class C> inline void induce_inplace(const C *s, int n, int K, const int *cnt, int m, int *sa, int *bkt) {
  std::fill(sa + m, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int p = sa[k];
    sa[k] = 0;
    sa[--bkt[s[p]]] = p;
  }
  induce_scan<false, Table>(s, n, K, cnt, sa, bkt);
}
// LMS の位置を後ろからの走査で見つけ、bits[p / 64] の bit (p % 64) に印を付けて m を返す。種類は配列に残さず、分岐もしない (64 個ごとに
// 1 語を書く)。Four なら位置 p ごとに q[4 * s[p] + 2 * (p が L 型) + (p - 1 が L 型)] を、そうでなければ q[s[p] + 1] を数える (位置 0 の
// 前は S 型とみなす)。f は L 型なら 1。LMS の位置を int の配列に持つより、段ごとに新しく確保する領域が小さく、CI の Linux で
// ページフォールトの分だけ速い。
template <bool Four, class C> inline int gather_lms(const C *s, int n, uint64_t *bits, int *q) {
  int m = 0, c0 = int(s[n - 1]), f0 = 1;  // 末尾の番兵は最も小さいので、最後の文字は L 型
  uint64_t w = 0;
  for (int p = n - 1; p > 0; --p) {  // 位置 p の種類 f0 と、p - 1 の種類 f1
    const int c1 = int(s[p - 1]), f1 = c1 > c0 - f0, is = f1 & (f0 ^ 1);
    ++q[Four ? 4 * c0 + 2 * f0 + f1 : c0 + 1];
    w |= uint64_t(is) << (p & 63), m += is;
    if ((p & 63) == 0) bits[p >> 6] = w, w = 0;
    c0 = c1, f0 = f1;
  }
  ++q[Four ? 4 * c0 + 2 * f0 : c0 + 1];
  bits[0] = w;  // 位置 0 は LMS にならない
  return m;
}
// 印を付けた LMS の位置を、位置の順に dst に書く。
inline void expand_lms(const uint64_t *bits, int n, int *dst) {
  for (int b = 0; b <= (n - 1) >> 6; ++b)
    for (uint64_t w = bits[b]; w; w &= w - 1) *dst++ = 64 * b + __builtin_ctzll(w);
}
// sort_lms_lr の区画の 1 回の走査。sa[i] を i = from から to の手前まで (Up なら前へ、でなければ後ろへ) 読み、1 つ前の接尾辞を置き先
// ptr[2c + t] (c はその文字、t はさらに 1 つ前が L 型なら 1) に置く (Up なら前から、でなければ後ろから)。読む要素の組の番号 d を
// 印を読むたびに 1 増やし、置く要素には、同じ置き先に最後に置いた要素と元の組が違えば印を付ける。D[2c + t] は置き先に最後に置いた
// 要素の、元の要素の組の番号。直前に使った置き先 lv の位置 (lb) と組の番号 (ld) はレジスタに置き、置き先が変わったときだけ表に書き
// 戻す。新しい d を返す。
template <bool Up, class C> inline int scan_lr(const C *s, int *sa, int from, int to, int *ptr, int *D, int d, int lv) {
  int *lb = sa + ptr[lv], ld = D[lv];
  for (int i = from; Up ? i < to : i > to; Up ? ++i : --i) {
    const int x = sa[i];
    d += x < 0;
    const int p = (x & POS) - 1, c = int(s[p]), v = 2 * c + (Up ? int(s[p - 1]) >= c : int(s[p - 1]) > c);
    if (v != lv) ptr[lv] = int(lb - sa), D[lv] = ld, lv = v, lb = sa + ptr[v], ld = D[v];
    const int y = p | int(unsigned(ld != d) << 31);
    if constexpr (Up) *lb++ = y; else *--lb = y;
    ld = d;
  }
  ptr[lv] = int(lb - sa), D[lv] = ld;
  return d;
}
// 文字の種類が少ない段で、LMS の部分文字列を並べて名前を付ける。q は gather_lms<true> で数えた 4 種類の数で、この中で書き換えて使う。
// lms は LMS の位置の列で、種を置くまでしか読まない。並んだ LMS を sa[0, m) に、位置 p の LMS の名前を印を付けて sa[m + (p / 2)] に
// 置き、名前の数を返す。
template <class C> inline int sort_lms_lr(const C *s, int n, int K, int *q, const int *lms, int m, int *sa) {
  // いちばん左の LMS (first) と、それより左の位置を数から外す。
  const int first = lms[0];
  {
    int c0 = int(s[first]), f0 = 0;
    for (int p = first; p > 0; --p) {
      const int c1 = int(s[p - 1]), f1 = c1 > c0 - f0;
      --q[4 * c0 + 2 * f0 + f1];
      c0 = c1, f0 = f1;
    }
    --q[4 * c0 + 2 * f0];
  }
  // 前から読む走査の置き先は lp[2c + t]、後ろから読む走査の置き先は rp[2c + t]。左の区画は文字ごとに [前が L 型の L 型 (lp[2c + 1]
  // から前へ) | 種 (後ろから)]、右の区画は文字ごとに [前が S 型の L 型 (lp[2c] から前へ) | 前も S 型の S 型 (rp[2c] から後ろへ)] で、
  // LMS は sa[0, m) に文字ごとに rp[2c + 1] から後ろへ置く。種は first のほかの LMS (lms は sa[n - m, n) にあってよい。左の区画は
  // sa[0, n - m) に収まる)。
  std::vector<int> ptr(4 * K);
  int *lp = ptr.data(), *rp = lp + 2 * K, left = 0;
  for (int c = 0; c < K; ++c) lp[2 * c + 1] = left, left += q[4 * c + 3] + q[4 * c + 1], rp[2 * c + 1] = left;
  for (int k = m - 1; k > 0; --k) sa[--rp[2 * int(s[lms[k]]) + 1]] = lms[k];
  // バケットごとに、いちばん前の種に組の印を付ける。種はバケットの中では 1 文字目だけで比べるので同じ組で、その前の L 型の組とも、
  // 前のバケットの組とも違う。
  for (int c = 0; c < K; ++c)
    if (q[4 * c + 1] > 0) sa[rp[2 * c + 1]] |= MARK;
  for (int c = 0, r = left + 1, l = 0, cf = int(s[first]); c < K; ++c) {
    lp[2 * c] = r, r += q[4 * c] + q[4 * c + 2], rp[2 * c] = r;
    l += q[4 * c + 1] + (c == cf), rp[2 * c + 1] = l;  // first も LMS として置く
  }
  int *D = q;  // 数はもう使わないので、組の番号の表に q の領域を使う
  std::fill(D, D + 2 * K, 0);
  const int c = int(s[n - 1]), lv = 2 * c + (int(s[n - 2]) >= c);  // 番兵から最後の文字の接尾辞を induce する
  sa[lp[lv]++] = (n - 1) | MARK, D[lv] = 1;
  const int d = scan_lr<true>(s, sa, 0, left, lp, D, 1, lv);
  // 右の区画の前が S 型の L 型の印を、バケットごとに組の先頭から組の末尾へ移す。各要素に 1 つ後ろの要素の印を付け、バケットの最後の
  // 要素には印を付ける。
  for (int c = 0; c < K; ++c) {
    int f = MARK;
    for (int i = lp[2 * c] - 1, lo = c ? rp[2 * c - 2] : left + 1; i >= lo; --i) {
      const int x = sa[i], y = (x & MARK) ^ f;
      f ^= y, sa[i] = x ^ y;
    }
  }
  scan_lr<false>(s, sa, n - first - 1, left, rp, D, d, 0);
  // 印は「前から見て次の LMS と違う組」を表すので、印のある LMS の次で名前を 1 増やす。
  std::fill(sa + m, sa + m + (n >> 1), 0);
  int names = 0;
  for (int k = 0; k < m; ++k) {
    const int x = sa[k];
    sa[m + ((x & POS) >> 1)] = names | MARK, names += x < 0;
  }
  return names;
}
// 文字の種類が多い段で、LMS の部分文字列を並べて名前を付ける。1 回目の induced sorting で LMS の部分文字列を並べ、長さと文字の並びが
// 同じなら同じ名前を付ける。最後の LMS の部分文字列は番兵を含むので、ほかのどれとも違う (長さを 0 とおく)。出し方は sort_lms_lr と同じ。
template <class C>
inline int sort_lms_wide(const C *s, int n, int K, const int *cnt, int *bkt, const int *lms, int m, int *sa) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) sa[--bkt[s[lms[k]]]] = lms[k];  // LMS の 1 つ前は L 型なので、正のまま置く
  induce_scan<true, false>(s, n, K, cnt, sa, bkt);
  for (int i = 0, k = 0; i < n; ++i) {  // 並んだ順に LMS の位置 (負の値で残っている) を sa[0, m) に詰める
    const int v = sa[i];
    sa[k] = ~v, k += v < 0;
  }
  std::fill(sa + m, sa + m + (n >> 1), 0);
  for (int j = 0; j + 1 < m; ++j) sa[m + (lms[j] >> 1)] = lms[j + 1] - lms[j] + 1;
  int names = 0;
  for (int k = 0, prev = 0, prev_len = 0; k < m; ++k) {
    const int p = sa[k], l = sa[m + (p >> 1)];
    int r = 0;
    if (l != 0 && l == prev_len)
      while (r < l && s[p + r] == s[prev + r]) ++r;
    names += r < l || l == 0;
    sa[m + (p >> 1)] = (names - 1) | MARK, prev = p, prev_len = l;
  }
  return names;
}
// s[0, n) (値は 0 以上 K 未満) の接尾辞配列を sa[0, n) に書く。
template <class C> void sa_is(const C *s, int n, int K, int *sa) {
  if (n < 2) {  // n が 2 以上だと分かる形にしておくと、gcc が std::fill の長さを負と疑わない
    if (n == 1) sa[0] = 0;
    return;
  }
  // cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。
  const bool wide = K > n / 32;
  std::vector<int> q(wide ? 0 : 4 * K, 0), cnt(K + 1, 0), bkt(K);
  std::unique_ptr<uint64_t[]> bits(new uint64_t[(n + 63) / 64]);
  const int m = wide ? gather_lms<false>(s, n, bits.get(), cnt.data()) : gather_lms<true>(s, n, bits.get(), q.data());
  for (int c = 0; c < K; ++c) cnt[c + 1] = wide ? cnt[c + 1] + cnt[c] : cnt[c] + q[4 * c] + q[4 * c + 1] + q[4 * c + 2] + q[4 * c + 3];
  if (m == 0) {  // すべて L 型なら、番兵からの induce だけで並ぶ
    induce_inplace<false>(s, n, K, cnt.data(), 0, sa, bkt.data());
    return;
  }
  // sort_lms_lr には LMS の位置を sa[n - m, n) に広げて渡し、sort_lms_wide は sa を 0 にしてから種を置くので、別の配列に広げて渡す。
  int names;
  if (wide) {
    std::unique_ptr<int[]> lms(new int[m]);
    expand_lms(bits.get(), n, lms.get());
    names = sort_lms_wide(s, n, K, cnt.data(), bkt.data(), lms.get(), m, sa);
  } else {
    expand_lms(bits.get(), n, sa + n - m);
    names = sort_lms_lr(s, n, K, q.data(), sa + n - m, m, sa);
  }
  if (names < m) {
    // 名前を文字列の順に sa[n - m, n) へ詰めて縮めた文字列にし (後ろから詰めるので、書く位置は読む位置より後ろ)、sa[0, m) を縮めた
    // 問題の接尾辞配列として解く。そのあと LMS の位置を sa[n - m, n) に広げて、縮めた問題の接尾辞配列を LMS の位置に直す。
    for (int i = m + (n >> 1) - 1, l = n; i >= m; --i) {
      const int x = sa[i];
      sa[l - 1] = x & POS, l -= x < 0;
    }
    sa_is<int>(sa + n - m, m, names, sa);
    expand_lms(bits.get(), n, sa + n - m);
    for (int k = 0; k < m; ++k) sa[k] = sa[n - m + sa[k]];
  } else {
    for (int k = 0; k < m; ++k) sa[k] &= POS;
  }
  // L 型の走査で置くたびにバケットが替わる割合は、並んだ LMS の隣どうしで 1 つ前の文字が違う割合でよく見積もれる。半分を超えたら表の形。
  int switched = 0;
  for (int k = 0; k < std::min(m - 1, 4096); ++k) switched += s[sa[k] - 1] != s[sa[k + 1] - 1];
  if (2 * switched > std::min(m - 1, 4096))
    induce_inplace<true>(s, n, K, cnt.data(), m, sa, bkt.data());
  else
    induce_inplace<false>(s, n, K, cnt.data(), m, sa, bkt.data());
}
}  // namespace sais_lr13

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_lr13::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
