#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
// P が 2 でも 5 でもないとき、S[i..j] が P の倍数なのは接尾辞 S[i..N-1] と S[j+1..N-1] の
// mod P が等しいとき。クエリ [fr, to] は接尾辞の値が等しい 2 つの位置の組を [fr-1, to] から
// 数えることになり、これは Mo で「同じ値の個数」を保てば C(cnt, 2) の増減で出る。
// P = 2, 5 のときは末尾の 1 桁だけで決まるので、桁ごとの累積和で直接数える。
int main() {
 cin.tie(0), ios::sync_with_stdio(0);
 long long P;
 string S;
 cin >> P >> S;
 int M;
 cin >> M;
 const int n= (int)S.size();
 vector<int> lo(M), hi(M);
 for(int i= 0; i < M; ++i) cin >> lo[i] >> hi[i];
 if(P == 2 || P == 5) {
  vector<int> cnt(n + 1, 0);
  vector<long long> sum(n + 1, 0);
  for(int j= 1; j <= n; ++j) {
   const int ok= ((S[j - 1] - '0') % P == 0);
   cnt[j]= cnt[j - 1] + ok;
   sum[j]= sum[j - 1] + (long long)j * ok;
  }
  for(int i= 0; i < M; ++i) {
   const int l= lo[i], r= hi[i];
   cout << (sum[r] - sum[l - 1]) - (long long)(l - 1) * (cnt[r] - cnt[l - 1]) << '\n';
  }
  return 0;
 }
 // suf[i] は S[i..N-1] の mod P。
 vector<long long> suf(n + 1, 0);
 long long pw= 1 % P;
 for(int i= n - 1; i >= 0; --i) {
  suf[i]= ((S[i] - '0') * pw + suf[i + 1]) % P;
  pw= pw * 10 % P;
 }
 vector<long long> vals(suf.begin(), suf.end());
 sort(vals.begin(), vals.end());
 vals.erase(unique(vals.begin(), vals.end()), vals.end());
 vector<int> id(n + 1);
 for(int i= 0; i <= n; ++i) id[i]= (int)(lower_bound(vals.begin(), vals.end(), suf[i]) - vals.begin());
 Mo mo;
 for(int i= 0; i < M; ++i) mo.query(lo[i] - 1, hi[i] + 1);  // 接尾辞の位置 [fr-1, to]
 vector<int> cnt(vals.size(), 0);
 vector<long long> ans(M);
 long long cur= 0;
 auto add= [&](int i) {
  const int x= id[i];
  cur+= cnt[x], ++cnt[x];
 };
 auto erase= [&](int i) {
  const int x= id[i];
  --cnt[x], cur-= cnt[x];
 };
 auto out= [&](int qid) { ans[qid]= cur; };
 mo.run(add, erase, out);
 for(int i= 0; i < M; ++i) cout << ans[i] << '\n';
}
