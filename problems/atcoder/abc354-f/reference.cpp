// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <vector>
#include <algorithm>
template <class T> std::pair<std::vector<int>, std::vector<std::vector<int>>> longest_increasing_subsequence(const std::vector<T> &a, bool strict= true) {
 int n= a.size();
 std::vector<int> idx(n);
 std::vector<T> dp(n);
 int len= 0;
 if (strict)
  for (int i= 0; i < n; ++i) {
   auto it= std::lower_bound(dp.begin(), dp.begin() + len, a[i]);
   if (*it= a[i]; (idx[i]= it - dp.begin()) == len) ++len;
  }
 else
  for (int i= 0; i < n; ++i) {
   auto it= std::upper_bound(dp.begin(), dp.begin() + len, a[i]);
   if (*it= a[i]; (idx[i]= it - dp.begin()) == len) ++len;
  }
 std::vector<std::vector<int>> cand(len);
 for (int i= n; i--;) {
  if (idx[i] == len - 1 || (!cand[idx[i] + 1].empty() && a[i] < a[cand[idx[i] + 1].back()])) cand[idx[i]].emplace_back(i);
  else idx[i]= -1;
 }
 for (auto &c: cand) std::reverse(c.begin(), c.end());
 return {idx, cand};
}
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int T;
 cin >> T;
 while(T--) {
  int N;
  cin >> N;
  vector<int> A(N);
  for(int i= 0; i < N; ++i) cin >> A[i];
  auto [idx, _]= longest_increasing_subsequence(A);
  vector<int> ans;
  for(int i= 0; i < N; ++i)
   if(idx[i] != -1) ans.push_back(i);
  int m= ans.size();
  cout << m << '\n';
  for(int i= 0; i < m; ++i) cout << ans[i] + 1 << " \n"[i + 1 == m];
 }
 return 0;
}
