#pragma once
// prefix doubling に、Larsson と Sadakane の手 (N. J. Larsson and K. Sadakane, Faster suffix sorting, 2007) を入れた版。先頭 h 文字が
// 同じ接尾辞のまとまりを、接尾辞配列の上の区間として持ち、まとまりの番号は区間の最後の位置にする。1 つだけのまとまりは位置が決まった
// ので、次の回から飛ばす (並べ終えた位置の並びは、先頭に負の長さを書いて 1 度で飛ばす)。2 つ以上のまとまりだけを、h 文字先の
// 接尾辞のまとまりの番号で並べ直して分け、h を倍にする。同じ回の中で先に分けたまとまりの番号を後のまとまりの鍵に使っても、番号の
// 順は崩れないので、全部を並べ終えるまで繰り返せばよい。最初は、使う文字を 1 から詰め直し、n + 1 を超えない範囲で先頭 r 文字を 1 つの
// 数に詰めて数え上げで並べ、h = r から始める。まとまりの中の並べ直しは、(h 文字先の番号, 添字) を u64 に詰め、番号だけを比べる
// 3 分割のクイックソートで行う。等しい番号は 1 回の分割でまとまるので、ほとんどの番号が同じまとまり (同じ文字が続く文字列) でも
// 1 回の分割で済む。論文のコードは写さず、手の考え方だけを使って書いた。
#include <algorithm>
#include <utility>
#include <vector>
#include "pj.hpp"

namespace sa_doubling_ls {
using u32= unsigned;
using u64= unsigned long long;
// a[0, m) を上位 32 bit (鍵) だけで並べる 3 分割のクイックソート。等しい鍵の並びの順は問わない。軸は、40 個を超えると 9 点の擬似
// 中央値、それ以下は 3 点の中央値にする。分割が 2 log m 段を超えたら std::sort に切り替えて、最悪でも O(m log m) にする
// (Library Checker の almost_single の 1 つは、3 点の中央値だけだと O(m^2) に落ちた)。小さい側を再帰で、大きい側をループで並べる。
inline u32 key_of(u64 x) { return u32(x >> 32); }
inline u32 med3(u32 x, u32 y, u32 z) { return std::max(std::min(x, y), std::min(std::max(x, y), z)); }
inline void sort_by_key(u64* a, int m, int depth) {
 while(m > 16) {
  if(depth-- == 0) {
   std::sort(a, a + m, [](u64 x, u64 y) { return key_of(x) < key_of(y); });
   return;
  }
  u32 v;
  if(m > 40) {
   const int t= m / 8;
   const auto k= [&](int i) { return key_of(a[i]); };
   v= med3(med3(k(0), k(t), k(2 * t)), med3(k(m / 2 - t), k(m / 2), k(m / 2 + t)), med3(k(m - 1 - 2 * t), k(m - 1 - t), k(m - 1)));
  } else {
   v= med3(key_of(a[0]), key_of(a[m / 2]), key_of(a[m - 1]));
  }
  int lt= 0, i= 0, gt= m;
  while(i < gt) {
   const u32 k= key_of(a[i]);
   if(k < v) std::swap(a[lt++], a[i++]);
   else if(k > v) std::swap(a[i], a[--gt]);
   else ++i;
  }
  if(lt < m - gt) sort_by_key(a, lt, depth), a+= gt, m-= gt;
  else sort_by_key(a + gt, m - gt, depth), m= lt;
 }
 for(int i= 1; i < m; ++i) {
  const u64 x= a[i];
  int j= i;
  for(; j > 0 && key_of(a[j - 1]) > key_of(x); --j) a[j]= a[j - 1];
  a[j]= x;
 }
}
inline void sort_by_key(u64* a, int m) {
 int log= 0;
 for(int t= m; t >>= 1;) ++log;
 sort_by_key(a, m, 2 * log);
}
inline void build(const string& s, vector<int>& out) {
 const int n= int(s.size());
 if(!n) return;
 // 使う文字を 1 から詰め直す (0 は末尾の番兵)。
 int code[256]= {};
 for(unsigned char c: s) code[c]= 1;
 int sigma= 1;
 for(int c= 0; c < 256; ++c)
  if(code[c]) code[c]= sigma++;
 // 先頭 r 文字を sigma 進数の 1 つの数に詰める。sigma^r <= n + 1 にして、数え上げの表を n + 1 個以下にする。
 int r= 1;
 long long pw= sigma;
 while(pw * sigma <= n + 1) pw*= sigma, ++r;
 vector<int> sa(n + 1), grp(n + 1);
 {
  const auto at= [&](int i) { return i < n ? code[(unsigned char)s[i]] : 0; };
  long long cur= 0;
  for(int t= 0; t < r; ++t) cur= cur * sigma + at(t);
  const long long top= pw / sigma;
  for(int i= 0; i < n; ++i) {
   grp[i]= int(cur);
   cur= (cur - at(i) * top) * sigma + at(i + r);
  }
  grp[n]= 0;  // 番兵は 1 つだけの最小の値
 }
 // 詰めた数を数え上げで並べる。まとまりの番号は区間の最後の位置。1 つだけのまとまりには並べ終えた印 (-1) を付ける。
 {
  vector<int> cnt(size_t(pw) + 1);
  for(int i= 0; i <= n; ++i) ++cnt[grp[i] + 1];
  for(size_t c= 1; c < cnt.size(); ++c) cnt[c]+= cnt[c - 1];
  vector<int> pos(cnt.begin(), cnt.end() - 1);
  for(int i= 0; i <= n; ++i) sa[pos[grp[i]]++]= i;
  for(int i= 0; i <= n; ++i) grp[i]= cnt[grp[i] + 1] - 1;
  for(size_t c= 0; c + 1 < cnt.size(); ++c)
   if(cnt[c + 1] - cnt[c] == 1) sa[cnt[c]]= -1;
 }
 vector<u64> tmp;
 for(long long h= r; sa[0] > -(n + 1); h*= 2) {
  int run= -1;  // 開いている並べ終えた並びの先頭 (-1 は無し)
  for(int x= 0; x <= n;) {
   if(sa[x] < 0) {
    if(run < 0) run= x;
    x-= sa[x];
    continue;
   }
   if(run >= 0) sa[run]= -(x - run), run= -1;
   // まとまり [x, g] を、h 文字先の接尾辞のまとまりの番号で並べ直す。鍵を先に全部読んでから番号を振り直す。
   const int g= grp[sa[x]], m= g - x + 1;
   tmp.resize(m);
   for(int j= 0; j < m; ++j) {
    const int i= sa[x + j];
    tmp[j]= u64(u32(grp[i + h])) << 32 | u32(i);
   }
   sort_by_key(tmp.data(), m);
   for(int j= 0; j < m;) {
    int e= j;
    while(e + 1 < m && (tmp[e + 1] >> 32) == (tmp[j] >> 32)) ++e;
    for(int t= j; t <= e; ++t) {
     const int i= int(u32(tmp[t]));
     sa[x + t]= i, grp[i]= x + e;
    }
    if(j == e) sa[x + j]= -1;  // 1 つだけのまとまり
    j= e + 1;
   }
   x= g + 1;
  }
  if(run >= 0) sa[run]= -(n + 1 - run);
 }
 // まとまりの番号は最後の位置なので、すべてが 1 つずつになれば接尾辞の位置そのもの。番兵 (位置 0) を除いて返す。
 for(int i= 0; i <= n; ++i) sa[grp[i]]= i;
 for(int j= 0; j < n; ++j) out[j]= sa[j + 1];
}
}  // namespace sa_doubling_ls

struct Solver {
  string s;
  vector<int> sa;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    sa.resize(s.size());
    sa_doubling_ls::build(s, sa);
  }

  const vector<int> &answer() const { return sa; }
};
