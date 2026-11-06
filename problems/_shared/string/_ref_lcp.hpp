// 文字列の基本の道具の族 (self/string/) の参照実装が使う、接尾辞どうしの LCP の素直な構造。接尾辞配列 (SA-IS、今の Library の
// SuffixArray と同じ手順)、Kasai の方法の LCP 配列、LCP 配列の sparse table からなる。速さは比べないので、どれも書いたままの形にした。
#pragma once
#include <algorithm>
#include <string>
#include <vector>
namespace string_ref {
inline std::vector<int> sa_is(const std::vector<int>& s, int K) {
 const int n= s.size();
 std::vector<char> t(n);
 std::vector<int> bkt(K, 0), bkt_l(K), bkt_r(K), sa(n), p1;
 t.back()= true;
 for(int i= n; --i;)
  if(t[i - 1]= (s[i - 1] < s[i] || (t[i] && s[i - 1] == s[i])); t[i] && !t[i - 1]) p1.push_back(i);
 std::reverse(p1.begin(), p1.end());
 const int n1= p1.size();
 for(int i= n; i--;) ++bkt[s[i]];
 for(int i= 0, sum= 0; i < K; ++i) sum+= bkt[i], bkt_r[i]= sum, bkt_l[i]= sum - bkt[i];
 std::vector<int> s1(n1), sa1(n1);
 std::fill_n(sa.begin(), n, -1), std::copy_n(bkt_r.begin(), K, bkt.begin());
 for(int i= n1; i--;) sa[--bkt[s[p1[i]]]]= p1[i];
 std::copy_n(bkt_l.begin(), K, bkt.begin());
 for(int i= 0, j; i < n; ++i)
  if((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt[s[j]]++]= j;
 std::copy_n(bkt_r.begin(), K, bkt.begin());
 for(int i= n, j; i--;)
  if((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt[s[j]]]= j;
 for(int i= 0, j= 0; i < n; ++i)
  if(t[sa[i]] && sa[i] > 0 && !t[sa[i] - 1]) sa1[j++]= sa[i];
 int name= 0;
 for(int i= 0, prev= -1, j, pos; i < n1; ++i, sa[pos]= name - 1)
  for(j= 0, pos= sa1[i];; ++j)
   if(prev == -1 || s[pos + j] != s[prev + j] || t[pos + j] != t[prev + j]) {
    prev= pos, ++name;
    break;
   } else if(j && ((t[pos + j] && !t[pos + j - 1]) || (t[prev + j] && !t[prev + j - 1]))) break;
 for(int i= n1; i--;) s1[i]= sa[p1[i]];
 if(name != n1) sa1= sa_is(s1, name);
 else
  for(int i= n1; i--;) sa1[s1[i]]= i;
 std::copy_n(bkt_r.begin(), K, bkt.begin()), std::fill_n(sa.begin(), n, -1);
 for(int i= n1; i--;) sa[--bkt[s[p1[sa1[i]]]]]= p1[sa1[i]];
 for(int i= 0, j; i < n; ++i)
  if((j= sa[i] - 1) >= 0 && !t[j]) sa[bkt_l[s[j]]++]= j;
 for(int i= n, j; i--;)
  if((j= sa[i] - 1) >= 0 && t[j]) sa[--bkt_r[s[j]]]= j;
 return sa;
}
// s の接尾辞配列。文字は unsigned char として比べる。
inline std::vector<int> suffix_array(const std::string& s) {
 std::vector<int> v(s.size() + 1, 0);
 for(size_t i= 0; i < s.size(); ++i) v[i]= int((unsigned char)s[i]) + 1;
 std::vector<int> sa= sa_is(v, 257);
 sa.erase(sa.begin());
 return sa;
}
// 接尾辞どうしの LCP。lcp(i, j) は s[i, n) と s[j, n) の最長共通接頭辞の長さ (0 <= i, j <= n)。
struct SuffixLCP {
 int n;
 std::vector<int> rnk;
 std::vector<std::vector<int>> st;  // st[k][i] は LCP 配列の [i, i + 2^k) の最小
 explicit SuffixLCP(const std::string& s): n(s.size()), rnk(s.size()) {
  if(n == 0) return;
  const std::vector<int> sa= suffix_array(s);
  for(int i= 0; i < n; ++i) rnk[sa[i]]= i;
  std::vector<int> lcp(std::max(n - 1, 1), 0);  // lcp[r] は sa[r] と sa[r + 1] の LCP
  for(int i= 0, h= 0; i < n; ++i) {
   if(h > 0) --h;
   if(rnk[i] == n - 1) {
    h= 0;
    continue;
   }
   const int j= sa[rnk[i] + 1];
   while(i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
   lcp[rnk[i]]= h;
  }
  st.push_back(lcp);
  for(int k= 1; (1 << k) <= int(lcp.size()); ++k) {
   const std::vector<int>& p= st.back();
   std::vector<int> q(lcp.size() - (1 << k) + 1);
   for(size_t i= 0; i < q.size(); ++i) q[i]= std::min(p[i], p[i + (1 << (k - 1))]);
   st.push_back(std::move(q));
  }
 }
 int lcp(int i, int j) const {
  if(i == n || j == n) return 0;
  if(i == j) return n - i;
  int l= rnk[i], r= rnk[j];
  if(l > r) std::swap(l, r);
  const int k= 31 - __builtin_clz(r - l);
  return std::min(st[k][l], st[k][r - (1 << k)]);
 }
};
}  // namespace string_ref
