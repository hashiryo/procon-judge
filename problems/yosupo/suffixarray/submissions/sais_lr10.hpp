#pragma once
// sais_lr9 の最後の induced sorting で、バケットの位置をレジスタに置く形と、置くたびに表を読み書きする形 (sais_lr8、libsais と同じ形) を、
// 並んだ LMS の隣どうしで 1 つ前の文字が違う割合で選ぶ版 (半分を超えたら表の形)。8573C の回では、表の形 (sais_lr8) は一様な文字列で
// 4 % から 5 % 速く、同じバケットに続けて置く並びが長い入力 (almost_single など) で遅かった。ほかは sais_lr9 と同じ。以下は sais_lr9 の説明。
// sais_lr7 を、同じ速さのまま小さく書き直した版。sais_lr7 は、文字の種類が多い段のために sais_z2 の sa_is をまるごと写していた
// (コメントを除いて 318 行)。この版では、LMS を集める走査、名前を詰めて縮めた文字列にする段、再帰、最後の induced sorting を 2 つの
// 道で共通にし、違うのは LMS の部分文字列を並べて名前を付ける段 (sort_lms_lr と sort_lms_wide) だけにする。1 回目と最後の induced
// sorting の走査も 1 つのテンプレート (induce_scan<First>) にまとめる。どちらの道も、並んだ LMS を sa[0, m) に、位置 p の LMS の
// 名前を印 (符号) を付けて sa[m + (p / 2)] に置いて終える。
// - sort_lms_lr (文字の種類が少ない段): libsais の組み立て方。位置を「自分が L 型か」と「1 つ前が L 型か」の 4 種類に分けて文字ごとに
//   数え、前から読む走査で読む要素 (LMS と、1 つ前も L 型の L 型) を左の区画に、後ろから読む走査で読む要素 (1 つ前が S 型の L 型と、
//   1 つ前も S 型の S 型) を右の区画に、文字ごとに隙間なく置く。置き先を「置く要素の文字と、その 1 つ前の種類」の 2 本のポインタで
//   決めるので、どちらの走査も区画の要素を順にすべて読み、空きの判定も induce するかの判定も要らない。いちばん左の LMS とそれより
//   左の位置は数から外して種にもしない。LMS の部分文字列の順位は同じ走査で符号の印を付けて決め、種にはバケットごとにいちばん前の
//   ものに印を付ける (付けないと、違う部分文字列に同じ名前が付いて再帰が 1 段増えることがある)。置き先の位置と組の番号は、直前に
//   使った置き先の分だけレジスタに置く。
// - sort_lms_wide (文字の種類が文字列の長さに比べて多い段、K > n / 32): sais_z2 と同じ解き方で、表は文字ごとに 1 本 (cnt と bkt)。
//   文字ごとに 4 本ずつの表は、max_random の再帰の 2 段目 (n = 160540、K = 90995) で数 MB になり、x64 の L2 に収まらない。
// 最後の induced sorting は、置く値の符号に induce するかを持たせる Yuta Mori の sais-lite の手で、今書いているバケットの位置は
// レジスタに置く。LMS の位置は、最初の走査で見つけて別の配列に残す。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <algorithm>
#include <memory>
#include <vector>
#include "pj.hpp"

namespace sais_lr10 {
constexpr int MARK = int(0x80000000u), POS = 0x7fffffff;
// バケットの末尾に LMS を置いた sa を、L 型、S 型の順に induced sorting する。cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。
// sa に置く値の符号に「この接尾辞の 1 つ前を、今の走査で induce するか」を持たせる。L 型を前から induce する走査で置く j は L 型なので、
// j - 1 が L 型かは s[j - 1] >= s[j] で決まり、S 型なら ~j を置く。走査で読んだ値は反転して、L 型の走査で induce しなかったもの
// (前が S 型) を、S 型を後ろから induce する走査で正の値として読む。空きは 0 にする (位置 0 の接尾辞と同じく、そこから induce
// しない)。今書いているバケットの位置はレジスタに置き、文字が変わったときだけ表に書き戻す。
// First (1 回目、LMS の部分文字列を並べる) では読んだ値を 0 に戻し、S 型の走査で前が L 型の j (LMS) は ~j で置いてそのまま残すので、
// 走査が終わると LMS だけが負の値で、LMS の部分文字列の順に残る。最後の induced sorting では、負の値はすべて反転されて接尾辞の位置に
// 戻る。
// Table (最後の induced sorting だけ) では、バケットの位置をレジスタに置かず、置くたびに表の bkt[c] を読み書きする (libsais と同じ形)。
// 置くたびにバケットが替わる入力 (一様な文字列) では、レジスタに置く形は替わるたびの書き戻しと分岐の分だけ遅く、替わることが少ない
// 入力 (同じバケットに続けて置く並びが長い) では、表を毎回読み書きする形はその待ちが積み重なる。
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
// order[0, m) (LMS の位置の列) を、その順を保って空の sa のバケットの末尾に置き、induced sorting する。
template <bool First, class C>
inline void induce(const C *s, int n, int K, const int *cnt, const int *order, int m, int *sa, int *bkt) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int d = order[k];
    sa[--bkt[s[d]]] = d;  // LMS の 1 つ前は L 型なので、正のまま置く
  }
  induce_scan<First, false>(s, n, K, cnt, sa, bkt);
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
// LMS の位置を後ろからの走査で見つけ、end の手前の [end - m, end) に位置の順に書いて m を返す。種類は配列に残さず、分岐もしない。
// Four なら位置 p ごとに q[4 * s[p] + 2 * (p が L 型) + (p - 1 が L 型)] を、そうでなければ q[s[p] + 1] を数える (位置 0 の前は S 型と
// みなす)。f は L 型なら 1。
template <bool Four, class C> inline int gather_lms(const C *s, int n, int *end, int *q) {
  int *w = end, m = 0;
  int c0 = int(s[n - 1]), f0 = 1;  // 末尾の番兵は最も小さいので、最後の文字は L 型
  for (int i = n - 2; i >= 0; --i) {
    const int c1 = int(s[i]), f1 = c1 > c0 - f0;
    ++q[Four ? 4 * c0 + 2 * f0 + f1 : c0 + 1];
    w[-1] = i + 1;
    const int is = f1 & (f0 ^ 1);
    w -= is, m += is;
    c0 = c1, f0 = f1;
  }
  ++q[Four ? 4 * c0 + 2 * f0 : c0 + 1];
  return m;
}
// 文字の種類が少ない段で、LMS の部分文字列を並べて名前を付ける (libsais の組み立て方)。q は gather_lms<true> で数えた 4 種類の数で、
// この中で書き換えて使う。並んだ LMS を sa[0, m) に、位置 p の LMS の名前を印を付けて sa[m + (p / 2)] に置き、名前の数を返す。
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
  // 前から読む走査の置き先は lp[2c + t]、後ろから読む走査の置き先は rp[2c + t] (c は置く要素の文字、t は置く要素の 1 つ前が L 型なら 1)。
  // 左の区画は文字ごとに [前が L 型の L 型 (lp[2c + 1] から前へ) | 種 (後ろから)]、右の区画は文字ごとに [前が S 型の L 型 (lp[2c] から
  // 前へ) | 前も S 型の S 型 (rp[2c] から後ろへ)] で、LMS は sa[0, m) に文字ごとに rp[2c + 1] から後ろへ置く。種は first のほかの LMS。
  std::vector<int> ptr(4 * K);
  int *lp = ptr.data(), *rp = lp + 2 * K;
  int left = 0;
  for (int c = 0; c < K; ++c) {
    lp[2 * c + 1] = left;
    left += q[4 * c + 3] + q[4 * c + 1];
    rp[2 * c + 1] = left;  // 種を後ろから置く位置 (種を置いたあとに LMS の置き場で書き直す)
  }
  for (int k = m - 1; k > 0; --k) {
    const int p = lms[k];
    sa[--rp[2 * int(s[p]) + 1]] = p;
  }
  // バケットごとに、いちばん前の種に組の印を付ける。種はバケットの中では 1 文字目だけで比べるので同じ組で、その前の L 型の組とも、
  // 前のバケットの組とも違う。
  for (int c = 0; c < K; ++c)
    if (q[4 * c + 1] > 0) sa[rp[2 * c + 1]] |= MARK;
  {
    const int cf = int(s[first]);
    int r = left + 1, l = 0;
    for (int c = 0; c < K; ++c) {
      lp[2 * c] = r;
      r += q[4 * c] + q[4 * c + 2];
      rp[2 * c] = r;
      l += q[4 * c + 1] + (c == cf);  // first も LMS として置く
      rp[2 * c + 1] = l;
    }
  }
  // 読む要素の組の番号 d を印を読むたびに 1 増やし、置く要素には、同じ置き先に最後に置いた要素と元の組が違えば印を付ける。D[2c + t]
  // は置き先 2c + t に最後に置いた要素の、元の要素の組の番号 (数はもう使わないので q の領域を使う)。直前に使った置き先 lv の位置 (lb)
  // と組の番号 (ld) はレジスタに置き、置き先が変わったときだけ表に書き戻す。
  int *D = q;
  std::fill(D, D + 2 * K, 0);
  int d = 0, lv, *lb, ld;
  {
    const int p = n - 1, c = int(s[p]);  // 番兵から最後の文字の接尾辞を induce する
    lv = 2 * c + (int(s[p - 1]) >= c), lb = sa + lp[lv];
    *lb++ = p | MARK;
    ld = ++d;
  }
  for (int i = 0; i < left; ++i) {
    const int x = sa[i];
    d += x < 0;
    const int p = (x & POS) - 1, c = int(s[p]), v = 2 * c + (int(s[p - 1]) >= c);
    if (v != lv) lp[lv] = int(lb - sa), D[lv] = ld, lv = v, lb = sa + lp[v], ld = D[v];
    *lb++ = p | int(unsigned(ld != d) << 31);
    ld = d;
  }
  lp[lv] = int(lb - sa), D[lv] = ld;
  // 右の区画の前が S 型の L 型の印を、バケットごとに組の先頭から組の末尾へ移す。各要素に 1 つ後ろの要素の印を付け、バケットの最後の
  // 要素には印を付ける。
  for (int c = 0; c < K; ++c) {
    const int lo = c ? rp[2 * c - 2] : left + 1;
    int f = MARK;
    for (int i = lp[2 * c] - 1; i >= lo; --i) {
      const int x = sa[i], y = (x & MARK) ^ f;
      f ^= y;
      sa[i] = x ^ y;
    }
  }
  lv = 0, lb = sa + rp[0], ld = D[0];
  for (int i = n - first - 1; i > left; --i) {
    const int x = sa[i];
    d += x < 0;
    const int p = (x & POS) - 1, c = int(s[p]), v = 2 * c + (int(s[p - 1]) > c);
    if (v != lv) rp[lv] = int(lb - sa), D[lv] = ld, lv = v, lb = sa + rp[v], ld = D[v];
    *--lb = p | int(unsigned(ld != d) << 31);
    ld = d;
  }
  // 印は「前から見て次の LMS と違う組」を表すので、印のある LMS の次で名前を 1 増やす。
  std::fill(sa + m, sa + m + (n >> 1), 0);
  int names = 0;
  for (int k = 0; k < m; ++k) {
    const int x = sa[k];
    sa[m + ((x & POS) >> 1)] = names | MARK;
    names += x < 0;
  }
  return names;
}
// 文字の種類が多い段で、LMS の部分文字列を並べて名前を付ける (sais_z2 と同じ解き方)。1 回目の induced sorting で LMS の部分文字列を
// 並べ、長さと文字の並びが同じなら同じ名前を付ける。最後の LMS の部分文字列は番兵を含むので、ほかのどれとも違う (長さを 0 とおく)。
// 出し方は sort_lms_lr と同じ。
template <class C>
inline int sort_lms_wide(const C *s, int n, int K, const int *cnt, int *bkt, const int *lms, int m, int *sa) {
  induce<true>(s, n, K, cnt, lms, m, sa, bkt);
  {
    int k = 0;  // 並んだ順に LMS の位置 (負の値で残っている) を sa[0, m) に詰める
    for (int i = 0; i < n; ++i) {
      const int v = sa[i];
      sa[k] = ~v;
      k += v < 0;
    }
  }
  std::fill(sa + m, sa + m + (n >> 1), 0);
  for (int j = 0; j + 1 < m; ++j) sa[m + (lms[j] >> 1)] = lms[j + 1] - lms[j] + 1;
  int names = 0;
  for (int k = 0, prev = 0, prev_len = 0; k < m; ++k) {
    const int p = sa[k], l = sa[m + (p >> 1)];
    bool diff = true;
    if (l != 0 && l == prev_len) {
      int r = 0;
      while (r < l && s[p + r] == s[prev + r]) ++r;
      diff = r < l;
    }
    names += diff;
    sa[m + (p >> 1)] = (names - 1) | MARK;
    prev = p, prev_len = l;
  }
  return names;
}
// s[0, n) (値は 0 以上 K 未満) の接尾辞配列を sa[0, n) に書く。
template <class C> void sa_is(const C *s, int n, int K, int *sa) {
  if (n < 2) {  // n が 2 以上だと分かる形にしておくと、gcc が下の std::fill の長さを負と疑わない
    if (n == 1) sa[0] = 0;
    return;
  }
  // cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。LMS の位置は lms[0, m) に位置の順に並べ、最後に縮めた問題の接尾辞配列を LMS の
  // 位置に直すまで残す (初期化しない配列で、m <= (n - 1) / 2)。
  const bool wide = K > n / 32;
  std::vector<int> q(wide ? 0 : 4 * K, 0), cnt(K + 1, 0), bkt(K);
  std::unique_ptr<int[]> buf(new int[n / 2 + 1]);
  int *const end = buf.get() + n / 2 + 1;
  const int m = wide ? gather_lms<false>(s, n, end, cnt.data()) : gather_lms<true>(s, n, end, q.data());
  const int *lms = end - m;
  for (int c = 0; c < K; ++c) cnt[c + 1] = wide ? cnt[c + 1] + cnt[c] : cnt[c] + q[4 * c] + q[4 * c + 1] + q[4 * c + 2] + q[4 * c + 3];
  if (m == 0) {  // すべて L 型なら、番兵からの induce だけで並ぶ
    induce<false>(s, n, K, cnt.data(), nullptr, 0, sa, bkt.data());
    return;
  }
  const int names = wide ? sort_lms_wide(s, n, K, cnt.data(), bkt.data(), lms, m, sa) : sort_lms_lr(s, n, K, q.data(), lms, m, sa);
  if (names < m) {
    // 名前を文字列の順に sa[n - m, n) へ詰めて縮めた文字列にし (後ろから詰めるので、書く位置は読む位置より後ろ)、sa[0, m) を縮めた
    // 問題の接尾辞配列として解く。そのあと、縮めた問題の接尾辞配列を lms で LMS の位置に直す。
    {
      int l = n;
      for (int i = m + (n >> 1) - 1; i >= m; --i) {
        const int x = sa[i];
        sa[l - 1] = x & POS;
        l -= x < 0;
      }
    }
    sa_is<int>(sa + n - m, m, names, sa);
    for (int k = 0; k < m; ++k) sa[k] = lms[sa[k]];
  } else {
    for (int k = 0; k < m; ++k) sa[k] &= POS;
  }
  // 最後の induced sorting の形を選ぶ。L 型の走査で置くたびにバケットが替わる割合は、並んだ LMS の隣どうしで 1 つ前の文字が違う割合で
  // よく見積もれる (先頭の 4096 組で、max_random の 1 段目は 0.96 で走査の実際は 0.92、binary_carry は 0.006 で実際は 0.0001)。
  int switched = 0;
  const int probe = std::min(m - 1, 4096);
  for (int k = 0; k < probe; ++k) switched += s[sa[k] - 1] != s[sa[k + 1] - 1];
  if (2 * switched > probe)
    induce_inplace<true>(s, n, K, cnt.data(), m, sa, bkt.data());
  else
    induce_inplace<false>(s, n, K, cnt.data(), m, sa, bkt.data());
}
}  // namespace sais_lr10

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_lr10::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
