#include <algorithm>
#include <array>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/MoOnTree.hpp"
using namespace std;
signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int N, Q;
 cin >> N >> Q;
 vector<int> x(N);
 for(auto& v: x) cin >> v;
 auto vals= x;
 sort(vals.begin(), vals.end()), vals.erase(unique(vals.begin(), vals.end()), vals.end());
 vector<int> y(N);
 for(int i= 0; i < N; ++i) y[i]= (int)(lower_bound(vals.begin(), vals.end(), x[i]) - vals.begin());
 MoOnTree mo(N);
 for(int i= 0; i < N - 1; ++i) {
  int a, b;
  cin >> a >> b, --a, --b;
  mo.add_edge(a, b);
 }
 vector<array<int, 3>> qs(Q);
 for(int i= 0; i < Q; ++i) {
  int v, w, l;
  cin >> v >> w >> l, --v, --w;
  qs[i]= {v, w, l};
  mo.query(v, w);
 }
 mo.build(0);
 const int V= (int)vals.size();
 int B= 1;
 while(B * B < V) ++B;
 vector<int> cnt(V, 0), bs((V + B - 1) / B, 0);
 auto toggle= [&](int v, bool entering) {
  const int c= y[v];
  if(entering) ++cnt[c], ++bs[c / B];
  else --cnt[c], --bs[c / B];
 };
 vector<int> ans(Q);
 auto out= [&](int id) {
  int k= qs[id][2], b= 0;
  while(k > bs[b]) k-= bs[b], ++b;
  int c= b * B, e= min(V, c + B);
  while(k > cnt[c]) k-= cnt[c], ++c;
  ans[id]= vals[c];
 };
 mo.run_hilbert(toggle, out);
 for(int i= 0; i < Q; ++i) cout << ans[i] << '\n';
}
