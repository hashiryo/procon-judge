#include <iostream>
#include <string>
#include "mylib/data_structure/SparseTable.hpp"
#include "mylib/string/SuffixTree.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 string S;
 cin >> S;
 SuffixArray sa(S);
 LCPArray lcp(sa);
 SuffixTree st(sa, lcp);
 SparseTable sa_mn(sa.sa, [&](int i, int j) { return min(i, j); });
 SparseTable sa_mx(sa.sa, [&](int i, int j) { return max(i, j); });
 int ans= 0;
 for(auto [l, r, h, hh]: st) {
  int i= sa_mn.prod(l, r), j= sa_mx.prod(l, r);
  ans= max(ans, min(j - i, hh));
 }
 cout << ans << '\n';
 return 0;
}
