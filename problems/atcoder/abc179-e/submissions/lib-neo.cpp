// lib.cpp を NeoLibrary の FunctionalGraph で書いたもの。値を位置の順に並べた累積和を作り、path が渡す区間と回数で足す。
#include <iostream>
#include <vector>
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 long long N, X, M;
 cin >> N >> X >> M;
 vector<int> to(M);
 for(long long i= 0; i < M; ++i) to[i]= i * i % M;
 FunctionalGraph g(to);
 vector<long long> sum(M + 1);
 for(int i= M; i--;) sum[g.to_seq(i) + 1]= i;
 for(int i= 0; i < M; ++i) sum[i + 1]+= sum[i];
 long long ans= 0;
 g.path(X, N - 1, [&](int l, int r, long long c) { ans+= (sum[r] - sum[l]) * c; });
 cout << ans << '\n';
 return 0;
}
