// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// https://atcoder.jp/contests/abc268/tasks/abc268_h

#include <iostream>
#include <string>
#include <vector>
#include <vector>
#include <algorithm>
#include <numeric>
template <class String> struct AhoCorasick {
 using symbol_t= typename String::value_type;
 AhoCorasick(const std::vector<String> &ps) {
  const size_t n= ps.size();
  std::vector<int> ord(n), rows;
  std::iota(ord.begin(), ord.end(), 0), std::sort(ord.begin(), ord.end(), [&](int l, int r) { return ps[l] < ps[r]; });
  std::vector<size_t> lcp(n, 0), prev(n, 0), cur(n);
  for (size_t i= 1, j, ed; i < n; lcp[i++]= j)
   for (j= 0, ed= std::min(ps[ord[i - 1]].size(), ps[ord[i]].size()); j < ed; j++)
    if (ps[ord[i - 1]][j] != ps[ord[i]][j]) break;
  size_t nodes= 1;
  for (size_t i= 0; i < n; i++) nodes+= ps[ord[i]].size() - lcp[i];
  bg.reserve(nodes + 1), es.reserve(nodes), match.reserve(nodes), rows.reserve(n + 1);
  for (size_t row= 0; row < n; row++)
   if (!ps[ord[row]].empty()) rows.push_back(row);
  rows.push_back(-1), bg.push_back(0), match.push_back({});
  for (size_t i= 0; i < n && ps[ord[i]].empty(); ++i) match[0].push_back(ord[i]);
  for (size_t col= 0; rows[0] != -1; col++) {
   int size= 0;
   for (int &r: rows) {
    if (r == -1) break;
    size_t row= r;
    if (size++; lcp[row] <= col) {
     if (size_t par= prev[row]; bg[par] == -1) bg[par]= es.size();
     es.push_back(ps[ord[row]][col]), bg.push_back(-1);
     if (match.push_back({}); col + 1 == ps[ord[row]].size())
      for (size_t i= row; i < n && ps[ord[i]] == ps[ord[row]]; ++i) match.back().push_back(ord[i]);
    }
    if (cur[row]= bg.size() - 1; col + 1 == ps[ord[row]].size()) r= -1;
   }
   *std::remove(rows.begin(), rows.begin() + size, -1)= -1, prev.swap(cur);
  }
  bg.push_back(es.size());
  for (size_t i= bg.size() - 1; --i;)
   if (bg[i] == -1) bg[i]= bg[i + 1];
  fail.assign(match.size(), -1);
  for (int u= 0, ed= match.size(); u < ed; u++)
   for (auto i= bg[u], v= i + 1; i < bg[u + 1]; i++, v++) {
    int r= fail[v]= transition(fail[u], es[i]);
    match[v].insert(match[v].end(), match[r].begin(), match[r].end());
   }
 }
 inline int initial_state() const { return 0; }
 inline std::vector<int> matched_patterns(int s) const { return match[s]; }
 inline bool is_accept(int s) const { return !match[s].empty(); }
 inline int transition(int s, symbol_t c) const {
  for (; s >= 0; s= fail[s])
   if (int v= next(s, c); v != -1) return v;
  return 0;
 }
 inline int state_size() const { return match.size(); }
private:
 std::vector<int> bg, fail;
 std::vector<symbol_t> es;
 std::vector<std::vector<int>> match;
 inline int next(int s, symbol_t c) const {
  int index= std::lower_bound(es.begin() + bg[s], es.begin() + bg[s + 1], c) - es.begin();
  if (index != bg[s + 1] && c == es[index]) return index + 1;
  return -1;
 }
};
using namespace std;
int main() {
 ios::sync_with_stdio(0);
 cin.tie(0);
 string S;
 int N;
 cin >> S >> N;
 vector<string> T(N);
 for(int i= 0; i < N; i++) cin >> T[i];
 AhoCorasick ac(T);
 int ans= 0;
 int s= ac.initial_state();
 for(char c: S) {
  s= ac.transition(s, c);
  if(ac.is_accept(s)) {
   ans++;
   s= ac.initial_state();
  }
 }
 cout << ans << '\n';
 return 0;
}
