#pragma once
// sais_lr3 の最後の induced sorting (induce_scan) で、読んだ要素から induce するか (値が正か) を分岐せずに扱う版。induce しない要素
// では、置く先を捨てる場所にし、文字を今のバケットの文字のままにしてバケットを替えない。バケットの位置は、これまでどおりレジスタに
// 置いて文字が変わったときだけ表に書き戻す。一様な文字列では induce する要素としない要素がばらばらに並び、分岐の予測を半分ほど外す。
// libsais の最後の induce にもこの分岐は残っている。ほかは sais_lr3 と同じ。以下は sais_lr3 の説明。
// sais_lr2 で、最初の走査で見つけた LMS の位置を別の配列に残し、種を置くときと、縮めた問題の接尾辞配列を LMS の位置に直すときに
// そこから読む版。sais_lr2 は LMS の位置を sa の後ろに書いていたので、再帰のあとに文字列をもう 1 回走査して集め直していた。手元の
// M2 では、その走査が binary_carry の全段の合計で 2 ms ほどかかっていた。ほかは sais_lr2 と同じ。以下は sais_lr2 の説明。
// sais_lr の 1 回目の induced sorting の 2 回の走査で、直前に使った置き先の位置と組の番号をレジスタに置き、置き先が変わったときだけ
// 表に書き戻す版。sais_lr は置くたびに置き先の位置と組の番号を表で読み書きするので、同じ置き先に続けて置く長い並び (almost_single)
// では、その読み書きの待ちが 1 要素ごとに積み重なる。ほかは sais_lr と同じ。以下は sais_lr の説明。
// sais_z の 1 回目の induced sorting と名前付けを、libsais の組み立て方で書き直した版 (先読みと手での展開は写さない)。最後の induced
// sorting (induce_scan、induce、induce_inplace) は sais_z と同じ。
// 1 回目の induced sorting では、位置 p を「p が L 型か」と「p - 1 が L 型か」の 4 種類に分けて文字ごとに数え、前から読む走査で読む要素
// (LMS と、1 つ前も L 型の L 型) を左の区画 sa[0, left) に、後ろから読む走査で読む要素 (1 つ前が S 型の L 型と、1 つ前も S 型の S 型)
// を右の区画 sa[left + 1, n - first) に、文字ごとに隙間なく置く。置き先は「置く要素の文字と、その 1 つ前の種類」で選ぶ 2 本のポインタで
// 決めるので、2 回の走査はどちらも区画の要素を順にすべて読み、空きの判定も induce するかの判定も要らない。いちばん左の LMS (first) と
// それより左の位置は、ほかの LMS の部分文字列に関わらないので数から外し、種にもしない。これで読む要素の 1 つ前と 2 つ前がいつもある。
// LMS の部分文字列の順位も同じ走査で付ける。読む要素の組の番号 d を符号の印を読むたびに 1 増やし、置く要素には、同じ置き先に最後に
// 置いた要素と元の組が違えば符号の印を付ける (sais_rank と同じ考えで、libsais と同じく種には印を付けない。種がバケットの直前の組と
// 同じ組になっても、その LMS どうしの順は、次の LMS の名前の比べ方で正しく決まる)。後ろから読む走査の前に、右の区画の印をバケット
// ごとに組の末尾へ移す。後ろから読む走査で LMS を sa[0, m) に置き終えると、印は「前から見て次の LMS と違う組」を表す。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <algorithm>
#include <memory>
#include <vector>
#include "pj.hpp"

namespace sais_lr4 {
constexpr int MARK = int(0x80000000u), POS = 0x7fffffff;
// バケットの末尾に LMS を置いた sa を、L 型、S 型の順に induced sorting する。cnt[c] はバケット c の
// 先頭、cnt[c + 1] は末尾の次。L 型と S 型の配列は読まない。sa に置く値の符号に「この接尾辞の 1 つ前を、今の走査で induce するか」を
// 持たせる (Yuta Mori の sais-lite の手)。L 型を前から induce する走査で置く j は L 型なので、j - 1 が L 型かは s[j - 1] >= s[j] で
// 決まり、S 型なら ~j を置く。走査で読んだ値は反転して、L 型の走査で induce しなかったもの (前が S 型) を、S 型を後ろから induce する
// 走査で正の値として読む。S 型の走査も同じで、j - 1 が L 型 (s[j - 1] > s[j]) なら ~j を置く。走査の最後に、負の値はすべて反転されて
// 接尾辞の位置に戻る。空きは 0 にする (位置 0 の接尾辞と同じく、そこから induce しない)。今書いているバケットの位置はレジスタに置き、
// 文字が変わったときだけ表に書き戻す。符号は分岐せずに決める (一様な文字列では前の種類がばらばらなので、分岐にすると予測を外す)。
template <class C> inline void induce_scan(const C *s, int n, int K, const int *cnt, int *sa, int *bkt) {
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c];
  int c1 = int(s[n - 1]);
  int *b = sa + bkt[c1];
  {
    const int j = n - 1;  // 番兵から最後の文字の接尾辞を induce する
    *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));
  }
  // induce するか (j > 0) は分岐せずに扱う。induce しない要素では、置く先を捨てる場所 (sink) にし、文字を今のバケットの文字のままにして
  // バケットを替えない。一様な文字列では induce する要素としない要素がばらばらに並ぶので、分岐にすると半分ほど予測を外す。
  int sink;
  for (int i = 0; i < n; ++i) {
    const int x = sa[i], ok = x > 0, j = (x - 1) & -ok;  // induce する接尾辞 (ok でなければ 0)
    sa[i] = ~x;
    const int c0 = c1 ^ ((int(s[j]) ^ c1) & -ok);
    if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
    int *w = ok ? b : &sink;
    *w = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));  // ~j は j ^ -1。j = 0 のときは s[0] を読んで捨てる
    b += ok;
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  c1 = 0;
  b = sa + bkt[0];
  for (int i = n - 1; i >= 0; --i) {
    const int x = sa[i], ok = x > 0, j = (x - 1) & -ok;
    sa[i] = x ^ (ok - 1);  // induce しない要素 (0 以下) は反転して接尾辞の位置に戻す
    const int c0 = c1 ^ ((int(s[j]) ^ c1) & -ok);
    if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
    b -= ok;
    int *w = ok ? b : &sink;
    *w = j ^ -int((j == 0) | (int(s[j - (j > 0)]) > c1));
  }
}
// order[0, m) (LMS の位置の列) を、その順を保って空の sa のバケットの末尾に置き、induced sorting する。
template <class C> inline void induce(const C *s, int n, int K, const int *cnt, const int *order, int m, int *sa, int *bkt) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int d = order[k];
    sa[--bkt[s[d]]] = d;  // LMS の 1 つ前は L 型なので、正のまま置く
  }
  induce_scan(s, n, K, cnt, sa, bkt);
}
// sa[0, m) に並んだ LMS の接尾辞を、その場でバケットの末尾へ移してから induced sorting する。k 番目に小さい LMS の行き先は k 以上
// なので、後ろから移せば、まだ移していないものを上書きしない。
template <class C> inline void induce_inplace(const C *s, int n, int K, const int *cnt, int m, int *sa, int *bkt) {
  std::fill(sa + m, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int p = sa[k];
    sa[k] = 0;
    sa[--bkt[s[p]]] = p;
  }
  induce_scan(s, n, K, cnt, sa, bkt);
}
// LMS の位置を後ろからの走査で見つけ、end の手前の [end - m, end) に位置の順に書いて m を返す。種類は配列に残さず、分岐もしない。q があれば、位置 p
// ごとに q[4 * s[p] + 2 * (p が L 型) + (p - 1 が L 型)] を数える (位置 0 の前は S 型とみなす)。f は L 型なら 1。
template <class C, bool Count> inline int gather_lms(const C *s, int n, int *end, int *q) {
  int *w = end, m = 0;
  int c0 = int(s[n - 1]), f0 = 1;  // 末尾の番兵は最も小さいので、最後の文字は L 型
  for (int i = n - 2; i >= 0; --i) {
    const int c1 = int(s[i]), f1 = c1 > c0 - f0;
    if constexpr (Count) ++q[4 * c0 + 2 * f0 + f1];
    w[-1] = i + 1;
    const int is = f1 & (f0 ^ 1);
    w -= is, m += is;
    c0 = c1, f0 = f1;
  }
  if constexpr (Count) ++q[4 * c0 + 2 * f0];
  return m;
}
// s[0, n) (値は 0 以上 K 未満) の接尾辞配列を sa[0, n) に書く。
template <class C> void sa_is(const C *s, int n, int K, int *sa) {
  if (n < 2) {  // n が 2 以上だと分かる形にしておくと、gcc が下の std::fill の長さを負と疑わない
    if (n == 1) sa[0] = 0;
    return;
  }
  // q[4c + 0] から q[4c + 3] は、文字 c の位置のうち、(S 型、前が S 型)、(S 型、前が L 型 = LMS)、(L 型、前が S 型)、(L 型、前が L 型)
  // の数。cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次 (最後の induced sorting で使う)。LMS の位置は lms[0, m) に位置の順に
  // 並べ、最後に縮めた問題の接尾辞配列を LMS の位置に直すまで残す (初期化しない配列で、m <= (n - 1) / 2)。
  std::vector<int> q(4 * K, 0), cnt(K + 1), bkt(K);
  std::unique_ptr<int[]> buf(new int[n / 2 + 1]);
  const int m = gather_lms<C, true>(s, n, buf.get() + n / 2 + 1, q.data());
  const int *lms = buf.get() + n / 2 + 1 - m;
  cnt[0] = 0;
  for (int c = 0; c < K; ++c) cnt[c + 1] = cnt[c] + q[4 * c] + q[4 * c + 1] + q[4 * c + 2] + q[4 * c + 3];
  if (m == 0) {  // すべて L 型なら、番兵からの induce だけで並ぶ
    induce(s, n, K, cnt.data(), nullptr, 0, sa, bkt.data());
    return;
  }
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
  int *D = q.data();  // D[2c + t] は置き先 2c + t に最後に置いた要素の、元の要素の組の番号 (数はもう使わないので q の領域を使う)
  std::fill(D, D + 2 * K, 0);
  // 直前に使った置き先 lv の位置 (lb) と組の番号 (ld) はレジスタに置き、置き先が変わったときだけ表に書き戻す。
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
  // sa[0, m) に並んだ LMS に名前を付ける。印は「前から見て次の LMS と違う組」を表すので、印のある LMS の次で名前を 1 増やす。LMS の
  // 位置 p の名前は sa[m + (p / 2)] に、置いた印 (符号) と一緒に置く (隣り合う LMS は 2 以上離れているので重ならない)。
  std::fill(sa + m, sa + m + (n >> 1), 0);
  int names = 0;
  for (int k = 0; k < m; ++k) {
    const int x = sa[k];
    sa[m + ((x & POS) >> 1)] = names | MARK;
    names += x < 0;
  }
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
  induce_inplace(s, n, K, cnt.data(), m, sa, bkt.data());
}
}  // namespace sais_lr4

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_lr4::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
