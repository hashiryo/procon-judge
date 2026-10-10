#pragma once
// sais_rank の印を決める番号の表のうち、今書いているバケットの 2 つを、バケットの位置と同じくレジスタに置く版。sais_rank は表を
// 置くたびに読み書きしていて、長い S 型の並びのように読む値が直前に書いた値になる並びで、その待ちが 1 要素ごとに積み重なり、
// 9V74 の x64-gcc で almost_single が sais_z より 2 割遅かった。ほかは sais_rank と同じ。以下は sais_rank の説明。
// sais_z の 1 回目の induced sorting の中で、LMS の部分文字列の順位も付ける版 (sais-lite の LMSsort2、libsais と同じ考え)。名前を付ける
// 段で LMS の部分文字列の長さを求めて文字を比べる手間 (長さと文字をばらばらの位置から読む) が無くなり、走査で印を数えるだけになる。
// 代わりに、induce の走査で置くたびに印を決める手間と、L 型の走査のあとに印を移す走査が 1 回増える。ほかは sais_z と同じ。以下は
// sais_z の説明。
// sais_ip の 1 回目の induced sorting で、読んだ値を 0 に戻し、LMS だけを負の値で残す版 (sais-lite の LMSsort1 と同じ考え)。並んだ
// LMS を集める段は符号を見るだけになり、L 型と S 型の配列をばらばらの位置から引かなくて済むので、種類の配列そのものを持たない。
// ほかは sais_ip と同じ。以下は sais_ip の説明。
// sais_inplace から、induced sorting の先読みを抜いた版。9V74 では先読みで 1 割遅くなっていた (1 段目の文字列は 0.5 MB で L2 に
// 収まり、ばらばらの読み込みがもともと L2 に当たるので、先読みの手間だけが増える)。ほかは sais_inplace と同じ。以下は sais_inplace の説明。
// sais_name の再帰で、並んだ LMS、長さと名前、縮めた文字列、縮めた問題の接尾辞配列を、すべて親の接尾辞配列の領域の中に置く版
// (sais-lite と同じ置き方)。並んだ LMS は sa[0, m) に、位置 p の長さと名前は sa[m + (p / 2)] に、縮めた文字列は sa[n - m, n) に詰め、
// 縮めた問題は sa[0, m) を自分の接尾辞配列として解く。段ごとに sorted、s1、sa1、長さの配列を確保しなくて済む。最後の induced
// sorting は、sa[0, m) に並んだ LMS をその場でバケットの末尾へ移してから行う。ほかは sais_name と同じ。以下は sais_name の説明。
// sais_pf の LMS の部分文字列に名前を付ける段で、種類を比べず、長さと文字の並びだけで比べる版 (ACL と同じ比べ方)。どちらも最後が
// LMS (S 型) なので、長さと文字が同じなら種類も同じになる。長さは次の LMS の位置から引いて (位置 / 2) 番目に置き、文字の並びは
// std::equal でまとめて比べる。ほかは sais_pf と同じ。以下は sais_pf の説明。
// sais_tag の induced sorting で、PF 個先の接尾辞の 1 つ前の文字を先読みする版 (libsais の手)。induced sorting は sa を順に読んだあと、
// s[sa[i] - 1] をばらばらの位置から読むのが重い。前からの走査では sa[i + PF]、後ろからの走査では sa[i - PF] を読み、その 1 つ前の
// 文字の場所を __builtin_prefetch で頼んでおく。先の値はまだ書き換わることがあるが、外れても先読みが無駄になるだけ。ほかは sais_tag と
// 同じ。以下は sais_tag の説明。
// sais の induced sorting で、L 型と S 型の配列を読まず、置く値の符号に induce するかを持たせる版 (Yuta Mori の sais-lite の手)。
// induced sorting は、接尾辞配列を順に読んだあと、L 型と S 型の配列と文字列をばらばらの位置から読む。種類の配列の読み込みを無くし、
// 種類は置くときに隣の 2 文字から決める。あわせて、今書いているバケットの位置をレジスタに置く。ほかは sais と同じ。以下は sais の説明。
// SA-IS を素直に書いた土台。ここから手を 1 つずつ足して比べる。1 段目は文字列の byte をそのまま文字として読み (int の列に写さない)、
// 末尾には、どの文字より小さい番兵があるものとして扱う (番兵を置かないので、最後に先頭を消す手間も無い)。L 型と S 型は 1 byte ずつの
// 配列に持つ。LMS の位置をバケットの末尾に置いてから、L 型を前から、S 型を後ろから induced sorting で並べ、並んだ LMS の部分文字列に
// 名前を付ける。名前がすべて違えば、そのまま LMS の接尾辞の順になり、同じ名前があれば名前の列で再帰する。名前は、隣り合う LMS の位置が
// 2 以上離れていることを使い、接尾辞配列の領域の (位置 / 2) 番目に置く。
// 接尾辞配列の実装を比べた記録は algo-notes の notes/suffix-array.md にある。
#include <algorithm>
#include <vector>
#include "pj.hpp"

namespace sais_rank2 {
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
  for (int i = 0; i < n; ++i) {
    int j = sa[i];
    sa[i] = ~j;
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
      --j;
      const int c0 = int(s[j]);
      if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
      *--b = j ^ -int((j == 0) | (int(s[j - (j > 0)]) > c1));
    } else {
      sa[i] = ~j;
    }
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
// 1 回目の induced sorting で付ける「組」の印。組は、LMS の部分文字列の並べ替えで同じ順位になる要素の並び。位置は 2^30 未満なので、
// 位置の bit 30 に印を持たせる (置く値の符号は induce するかに使っている)。
constexpr int GROUP = 1 << 30, POS = GROUP - 1;
// 置く j の組を決めて、印を付けた値を返す。D[2 * c + tag] は、バケット c に tag の側で最後に置いた要素の、元の要素の組の番号。元の組が
// 変われば、置く要素も前の要素と違う組になる。今の走査で読む側 (tag = 0) と次の走査で読む側 (tag = 1) は読む順が別なので、分けて持つ。
// 今書いているバケットの 2 つの番号は、バケットの位置と同じくレジスタ (g0 が tag = 0 の側、g1 が tag = 1 の側) に置き、文字が
// 変わったときだけ D に書き戻す。長い S 型の並びのように、読む値が直前に書いた値になる並びで、番号の読み書きの待ちが 1 要素ごとに
// 積み重ならないようにする (sais_rank は表を毎回読み書きして、almost_single で 2 割遅くなった)。
inline int put_group(int &g0, int &g1, int tag, int d, int j) {
  const int f = (tag ? g1 : g0) != d;
  g0 = tag ? g0 : d;
  g1 = tag ? d : g1;
  return (j | f << 30) ^ -tag;
}
// 1 回目の induced sorting (LMS の部分文字列を並べる)。order[0, m) を induce と同じくバケットの末尾に置き、induce_scan と同じ符号の
// 手で並べるが、読んだ値は 0 に戻す。S 型を後ろから induce する走査で、前が L 型の j (LMS) は ~j で置いてそのまま残すので、走査が
// 終わると LMS だけが負の値で、LMS の部分文字列の順に残る。位置 0 の接尾辞は、そこから induce するものが無い。
// あわせて、LMS の部分文字列の順位も付ける (sais-lite の LMSsort2、libsais と同じ考え)。読む要素の組の番号 d を、印を読むたびに 1 増やし、
// 置く要素には、同じバケットの同じ側に最後に置いた要素と元の組が違えば印を付ける。前から読む走査では、印は組の先頭に付く。後ろから
// 読む走査のために、L 型の走査のあとで、残っている要素 (前が S 型の L 型) の印を組の末尾へ移す。S 型の走査で置く印は、置いた順
// (後ろから) で組の先頭に付くので、走査が終わると、LMS の印は「前から見て次の LMS と違う組」を表す。
template <class C> inline void induce_lms(const C *s, int n, int K, const int *cnt, const int *order, int m, int *sa, int *bkt, int *D) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int d = order[k];
    sa[--bkt[s[d]]] = d;
  }
  // バケットの中の LMS は 1 文字目だけで比べるので同じ組で、その前の L 型とは違う組。バケットごとに、いちばん前の LMS に印を付ける。
  for (int c = 0; c < K; ++c)
    if (bkt[c] != cnt[c + 1]) sa[bkt[c]] |= GROUP;
  std::fill(D, D + 2 * K, -1);
  int d = 0;  // 番兵の組
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c];
  int c1 = int(s[n - 1]);
  int *b = sa + bkt[c1];
  int g0 = D[2 * c1], g1 = D[2 * c1 + 1];
  {
    const int j = n - 1;  // 番兵から最後の文字の接尾辞を induce する
    *b++ = put_group(g0, g1, (j > 0) & (int(s[j - (j > 0)]) < c1), d, j);
  }
  for (int i = 0; i < n; ++i) {
    const int v = sa[i];
    sa[i] = ~v & (v >> 31);  // 前が S 型で、この走査では induce しないもの (負の値) は正に戻し、ほかは 0 にする
    if (v > 0) {
      d += v >> 30;
      int j = v & POS;
      if (j > 0) {
        --j;
        const int c0 = int(s[j]);
        if (c0 != c1) {
          bkt[c1] = int(b - sa), D[2 * c1] = g0, D[2 * c1 + 1] = g1;
          c1 = c0, b = sa + bkt[c1], g0 = D[2 * c1], g1 = D[2 * c1 + 1];
        }
        *b++ = put_group(g0, g1, (j > 0) & (int(s[j - (j > 0)]) < c1), d, j);
      }
    }
  }
  D[2 * c1] = g0, D[2 * c1 + 1] = g1;
  // 残っている要素の印を、組の先頭から組の末尾へ移す。後ろから読んで、各要素に 1 つ後ろの要素の印を付け、いちばん後ろの要素には
  // 印を付ける。各バケットで最初に置いた要素には印が付いているので、組はバケットをまたがない。
  {
    int carry = GROUP;
    for (int i = n - 1; i >= 0; --i) {
      const int v = sa[i];
      const int x = ((v & GROUP) ^ carry) & -int(v != 0);
      carry ^= x;
      sa[i] = v ^ x;
    }
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  ++d;  // L 型の走査で D に書いたどの番号とも違う番号から始め、各バケットで最初に置く要素に印が付くようにする
  c1 = 0;
  b = sa + bkt[0];
  g0 = D[0], g1 = D[1];
  for (int i = n - 1; i >= 0; --i) {
    const int v = sa[i];
    if (v > 0) {
      sa[i] = 0;
      d += v >> 30;
      int j = v & POS;
      if (j > 0) {
        --j;
        const int c0 = int(s[j]);
        if (c0 != c1) {
          bkt[c1] = int(b - sa), D[2 * c1] = g0, D[2 * c1 + 1] = g1;
          c1 = c0, b = sa + bkt[c1], g0 = D[2 * c1], g1 = D[2 * c1 + 1];
        }
        *--b = put_group(g0, g1, (j > 0) & (int(s[j - (j > 0)]) > c1), d, j);
      }
    }
  }
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
// s[0, n) (値は 0 以上 K 未満) の接尾辞配列を sa[0, n) に書く。
template <class C> void sa_is(const C *s, int n, int K, int *sa) {
  if (n == 0) return;
  if (n == 1) {
    sa[0] = 0;
    return;
  }
  // ti は位置 i が S 型なら 1。末尾の番兵は最も小さいので、最後の文字は L 型。種類は配列に残さず、LMS の位置だけを後ろからの 1 回の
  // 走査で分岐せずに決める (一様な文字列では L 型と S 型がばらばらに並ぶので、分岐にすると予測を外す)。LMS の位置は後ろから書いて、
  // 最後に前へ詰める。
  std::vector<int> lms(n / 2 + 1);
  int m = 0;
  {
    int *w = lms.data() + lms.size();
    unsigned char ti = 0;
    for (int i = n - 1; i > 0; --i) {
      const unsigned char tp = (s[i - 1] < s[i]) | ((s[i - 1] == s[i]) & ti);
      w[-1] = i;
      const int is = ti & (tp ^ 1);
      w -= is, m += is;
      ti = tp;
    }
    std::copy(w, w + m, lms.data());
    lms.resize(m);
  }
  // cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。
  std::vector<int> cnt(K + 1, 0), head(K);
  for (int i = 0; i < n; ++i) ++cnt[int(s[i]) + 1];
  for (int c = 0; c < K; ++c) cnt[c + 1] += cnt[c];
  if (m == 0) {  // すべて L 型なら、番兵からの induce だけで並ぶ
    induce(s, n, K, cnt.data(), lms.data(), 0, sa, head.data());
    return;
  }
  {
    std::vector<int> D(2 * K);
    induce_lms(s, n, K, cnt.data(), lms.data(), m, sa, head.data(), D.data());
  }
  // 並んだ LMS の部分文字列に名前を付ける。LMS の部分文字列は、次の LMS の位置まで (番兵を含むものは番兵まで)。
  // 並んだ順に LMS (負の値で、印を付けて残っている) を sa[0, m) に詰め、印の数を数える (書く位置は読む位置より前なので、その場で
  // 詰めてよい)。印は前から見て次の LMS と違う組を表し、いちばん後ろの LMS にも付くので、印の数が名前の数になる。
  int names = 0;
  {
    int k = 0;
    for (int i = 0; i < n; ++i) {
      const int v = sa[i];
      sa[k] = ~v;
      k += v < 0;
      names += (v < 0) & (~v >> 30);
    }
  }
  // LMS の位置 p の名前は sa[m + (p / 2)] に置く (隣り合う LMS は 2 以上離れているので重ならず、m + n / 2 <= n に収まる)。後ろから
  // 読んで、印を読むたびに名前を 1 減らす。
  std::fill(sa + m, sa + n, -1);
  {
    int name = names;
    for (int k = m - 1; k >= 0; --k) {
      const int v = sa[k], p = v & POS;
      name -= v >> 30;
      sa[k] = p;
      sa[m + (p >> 1)] = name;
    }
  }
  // 名前を文字列の順に sa[n - m, n) へ詰めて、縮めた文字列にする (後ろから詰めるので、書く位置は読む位置より後ろ)。
  {
    int j = n;
    for (int i = n - 1; i >= m; --i) {
      const int v = sa[i];
      sa[j - 1] = v;
      j -= v >= 0;
    }
  }
  int *s1 = sa + n - m, *sa1 = sa;  // m <= n / 2 なので重ならない
  if (names < m) sa_is<int>(s1, m, names, sa1);
  else
    for (int j = 0; j < m; ++j) sa1[s1[j]] = j;
  for (int k = 0; k < m; ++k) sa[k] = lms[sa1[k]];
  induce_inplace(s, n, K, cnt.data(), m, sa, head.data());
}
}  // namespace sais_rank2

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_rank2::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
