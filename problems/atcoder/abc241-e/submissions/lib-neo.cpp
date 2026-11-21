// lib.cpp を NeoLibrary の FunctionalGraph で書いたもの。値を位置の順に並べた累積和を作り、path が渡す区間と回数で足す。
#include <iostream>
#include <vector>
#include "neo/base/ds/tree/FunctionalGraph.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 long long K;
 cin >> N >> K;
 vector<long long> A(N);
 for(int i= 0; i < N; ++i) cin >> A[i];
 vector<int> to(N);
 for(int i= 0; i < N; ++i) to[i]= (i + A[i]) % N;
 FunctionalGraph g(to);
 vector<long long> sum(N + 1);
 for(int i= N; i--;) sum[g.to_seq(i) + 1]= A[i];
 for(int i= 0; i < N; ++i) sum[i + 1]+= sum[i];
 long long ans= 0;
 g.path(0, K - 1, [&](int l, int r, long long c) { ans+= c * (sum[r] - sum[l]); });
 cout << ans << '\n';
 return 0;
}
