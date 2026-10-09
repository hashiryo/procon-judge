#pragma once
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

namespace sais_pf {
// order[0, m) (LMS の位置の列) を、その順を保ってバケットの末尾に置き、L 型、S 型の順に induced sorting する。cnt[c] はバケット c の
// 先頭、cnt[c + 1] は末尾の次。L 型と S 型の配列は読まない。sa に置く値の符号に「この接尾辞の 1 つ前を、今の走査で induce するか」を
// 持たせる (Yuta Mori の sais-lite の手)。L 型を前から induce する走査で置く j は L 型なので、j - 1 が L 型かは s[j - 1] >= s[j] で
// 決まり、S 型なら ~j を置く。走査で読んだ値は反転して、L 型の走査で induce しなかったもの (前が S 型) を、S 型を後ろから induce する
// 走査で正の値として読む。S 型の走査も同じで、j - 1 が L 型 (s[j - 1] > s[j]) なら ~j を置く。走査の最後に、負の値はすべて反転されて
// 接尾辞の位置に戻る。空きは 0 にする (位置 0 の接尾辞と同じく、そこから induce しない)。今書いているバケットの位置はレジスタに置き、
// 文字が変わったときだけ表に書き戻す。符号は分岐せずに決める (一様な文字列では前の種類がばらばらなので、分岐にすると予測を外す)。
// 先読みする距離 (要素の数)。
constexpr int PF = 32;
template <class C> inline void induce(const C *s, int n, int K, const int *cnt, const int *order, int m, int *sa, int *bkt) {
  std::fill(sa, sa + n, 0);
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c + 1];
  for (int k = m - 1; k >= 0; --k) {
    const int d = order[k];
    sa[--bkt[s[d]]] = d;  // LMS の 1 つ前は L 型なので、正のまま置く
  }
  for (int c = 0; c < K; ++c) bkt[c] = cnt[c];
  int c1 = int(s[n - 1]);
  int *b = sa + bkt[c1];
  {
    const int j = n - 1;  // 番兵から最後の文字の接尾辞を induce する
    *b++ = j ^ -int((j > 0) & (int(s[j - (j > 0)]) < c1));
  }
  for (int i = 0; i < n; ++i) {
    if (i + PF < n) {
      const int q = sa[i + PF];
      __builtin_prefetch(s + (q > 0 ? q - 1 : 0));
    }
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
    if (i >= PF) {
      const int q = sa[i - PF];
      __builtin_prefetch(s + (q > 0 ? q - 1 : 0));
    }
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
// s[0, n) (値は 0 以上 K 未満) の接尾辞配列を sa[0, n) に書く。
template <class C> void sa_is(const C *s, int n, int K, int *sa) {
  if (n == 0) return;
  if (n == 1) {
    sa[0] = 0;
    return;
  }
  // t[i] は S 型なら 1。末尾の番兵は最も小さいので、最後の文字は L 型。種類と LMS の位置は、後ろからの 1 回の走査で分岐せずに決める
  // (一様な文字列では L 型と S 型がばらばらに並ぶので、分岐にすると予測を外す)。LMS の位置は後ろから書いて、最後に前へ詰める。
  std::vector<unsigned char> t(n);
  std::vector<int> lms(n / 2 + 1);
  t[n - 1] = 0;
  int m = 0;
  {
    int *w = lms.data() + lms.size();
    for (int i = n - 1; i > 0; --i) {
      const unsigned char ti = t[i], tp = (s[i - 1] < s[i]) | ((s[i - 1] == s[i]) & ti);
      t[i - 1] = tp;
      w[-1] = i;
      const int is = ti & (tp ^ 1);
      w -= is, m += is;
    }
    std::copy(w, w + m, lms.data());
    lms.resize(m);
  }
  const auto is_lms = [&](int i) { return i > 0 && t[i] && !t[i - 1]; };
  // cnt[c] はバケット c の先頭、cnt[c + 1] は末尾の次。
  std::vector<int> cnt(K + 1, 0), head(K);
  for (int i = 0; i < n; ++i) ++cnt[int(s[i]) + 1];
  for (int c = 0; c < K; ++c) cnt[c + 1] += cnt[c];
  induce(s, n, K, cnt.data(), lms.data(), m, sa, head.data());
  if (m == 0) return;  // すべて L 型なら、番兵からの induce だけで並んでいる
  // 並んだ LMS の部分文字列に名前を付ける。LMS の部分文字列は、次の LMS の位置まで (番兵を含むものは番兵まで)。
  // 並んだ順に LMS の位置を集める。これも分岐せずに書いて、LMS のときだけ書く位置を進める。
  std::vector<int> sorted(m + 1);
  {
    int k = 0;
    for (int i = 0; i < n; ++i) {
      const int v = sa[i];
      sorted[k] = v;
      k += v > 0 && (t[v] & (t[v - 1] ^ 1));
    }
    sorted.resize(m);
  }
  const auto same = [&](int a, int b) {
    for (int k = 0;; ++k) {
      const int x = a + k, y = b + k;
      if (x == n || y == n) return false;  // 番兵は 1 つだけ
      if (s[x] != s[y] || t[x] != t[y]) return false;
      if (k > 0 && is_lms(x)) return true;  // 種類が同じなので、y も LMS
    }
  };
  int names = 0;
  for (int k = 0; k < m; ++k) {
    if (k == 0 || !same(sorted[k - 1], sorted[k])) ++names;
    sa[sorted[k] >> 1] = names - 1;
  }
  std::vector<int> s1(m), sa1(m);
  for (int j = 0; j < m; ++j) s1[j] = sa[lms[j] >> 1];
  if (names < m) sa_is<int>(s1.data(), m, names, sa1.data());
  else
    for (int j = 0; j < m; ++j) sa1[s1[j]] = j;
  for (int k = 0; k < m; ++k) sorted[k] = lms[sa1[k]];
  induce(s, n, K, cnt.data(), sorted.data(), m, sa, head.data());
}
}  // namespace sais_pf

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sais_pf::sa_is(reinterpret_cast<const unsigned char *>(s.data()), int(s.size()), 256, sa.data());
  }

  const vector<int> &answer() const { return sa; }
};
