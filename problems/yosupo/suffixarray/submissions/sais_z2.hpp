#pragma once
// sais_z に、sais_h4 の数え方 (表が文字列より十分小さいときは 4 本の表に分けて数える) と、sais_eq の比べ方 (長さが同じ LMS の
// 部分文字列を std::equal でなくその場のループで比べる) を重ねた版。9V74 の回では、sais_h4 は arm でだけ効き、sais_eq は x64-clang
// の binary_carry で 5 % ほど縮んだ。ほかは sais_z と同じ。以下は sais_z の説明。
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

namespace sais_z2 {
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
// 1 回目の induced sorting (LMS の部分文字列を並べる)。order[0, m) を induce と同じくバケットの末尾に置き、induce_scan と同じ符号の
// 手で並べるが、読んだ値は 0 に戻す。S 型を後ろから induce する走査で、前が L 型の j (LMS) は ~j で置いてそのまま残すので、走査が
// 終わると LMS だけが負の値で、LMS の部分文字列の順に残る。位置 0 の接尾辞は、そこから induce するものが無いので 0 (空き) として置く。
template <class C> inline void induce_lms(const C *s, int n, int K, const int *cnt, const int *order, int m, int *sa, int *bkt) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int d = order[k];
    sa[--bkt[s[d]]] = d;
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c];
  int c1 = int(s[n - 1]);
  int *b = sa + bkt[c1];
  {
    const int j = n - 1;  // 番兵から最後の文字の接尾辞を induce する
    *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));
  }
  for (int i = 0; i < n; ++i) {
    int j = sa[i];
    sa[i] = ~j & (j >> 31);  // 前が S 型で、この走査では induce しないもの (負の値) は正に戻し、ほかは 0 にする
    if (j > 0) {
      --j;
      const int c0 = int(s[j]);
      if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
      *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));
    }
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  c1 = 0;
  b = sa + bkt[0];
  for (int i = n - 1; i >= 0; --i) {
    int j = sa[i];
    if (j > 0) {
      sa[i] = 0;
      --j;
      const int c0 = int(s[j]);
      if (c0 != c1) bkt[c1] = int(b - sa), b = sa + bkt[c1 = c0];
      *--b = j ^ -int((j > 0) & (int(s[j - (j > 0)]) > c1));
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
// cnt[c] に文字 c の数を足す。同じ文字が続くと、1 つの数への足し込みが前の足し込みの書き込みを待つので、表が文字列より十分小さい
// ときは、4 本の表に分けて数えてから足し合わせる (sais_h4 と同じ)。
template <class C> inline void count_chars(const C *s, int n, int K, int *cnt) {
  int i = 0;
  if (16 * K <= n) {
    std::vector<int> w(3 * K, 0);
    int *w1 = w.data(), *w2 = w1 + K, *w3 = w2 + K;
    for (; i + 4 <= n; i += 4) ++cnt[s[i]], ++w1[s[i + 1]], ++w2[s[i + 2]], ++w3[s[i + 3]];
    for (int c = 0; c < K; ++c) cnt[c] += w1[c] + w2[c] + w3[c];
  }
  for (; i < n; ++i) ++cnt[s[i]];
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
  count_chars(s, n, K, cnt.data() + 1);
  for (int c = 0; c < K; ++c) cnt[c + 1] += cnt[c];
  if (m == 0) {  // すべて L 型なら、番兵からの induce だけで並ぶ
    induce(s, n, K, cnt.data(), lms.data(), 0, sa, head.data());
    return;
  }
  induce_lms(s, n, K, cnt.data(), lms.data(), m, sa, head.data());
  // 並んだ LMS の部分文字列に名前を付ける。LMS の部分文字列は、次の LMS の位置まで (番兵を含むものは番兵まで)。
  // 並んだ順に LMS の位置 (負の値で残っている) を sa[0, m) に詰める (書く位置は読む位置より前なので、その場で詰めてよい)。
  {
    int k = 0;
    for (int i = 0; i < n; ++i) {
      const int v = sa[i];
      sa[k] = ~v;
      k += v < 0;
    }
  }
  // LMS の位置 p の長さと名前は sa[m + (p / 2)] に置く (隣り合う LMS は 2 以上離れているので重ならず、m + n / 2 <= n に収まる)。
  // 長さと文字の並びが同じなら同じ部分文字列 (sais_name と同じ比べ方)。最後の LMS の部分文字列は番兵を含むので、ほかのどれとも違う。
  std::fill(sa + m, sa + n, -1);
  for (int j = 0; j + 1 < m; ++j) sa[m + (lms[j] >> 1)] = lms[j + 1] - lms[j] + 1;
  const int last = lms[m - 1];
  sa[m + (last >> 1)] = 0;
  int names = 0;
  {
    int prev = -1, prev_len = 0;
    for (int k = 0; k < m; ++k) {
      const int p = sa[k], l = sa[m + (p >> 1)];
      bool diff = true;
      if (prev >= 0 && p != last && prev != last && l == prev_len) {
        int q = 0;
        while (q < l && s[p + q] == s[prev + q]) ++q;
        diff = q < l;
      }
      names += diff;
      sa[m + (p >> 1)] = names - 1;
      prev = p, prev_len = l;
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
}  // namespace sais_z2

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_z2::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
