// LOJ 2874「JOISC 2014 Day1」历史研究。区間 [A,B] の答は max_t t・(区間で t が現れる回数)。
// 最大値は削除で戻せないので、追加だけをしてクエリごとに一時的に足したぶんを巻き戻す
// 回滚 Mo (RollbackMo) で解く。値は座標圧縮して種類ごとの個数を持ち、現在の最大値を
// 更新する。undo は直前に足した 1 つを外すので、足す前の最大値をスタックに積んでおき、
// 外すときに戻す (最大値は単調にしか上がらないため、これで正しく戻る)。
#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/base/engine/query/RollbackMo.hpp"
using namespace std;
using i64 = long long;

signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n, q;
 cin >> n >> q;
 vector<i64> x(n);
 for(auto& v: x) cin >> v;
 vector<i64> xs(x);
 sort(xs.begin(), xs.end());
 xs.erase(unique(xs.begin(), xs.end()), xs.end());
 vector<int> id(n);
 for(int i= 0; i < n; ++i) id[i]= (int)(lower_bound(xs.begin(), xs.end(), x[i]) - xs.begin());
 RollbackMo mo;
 for(int i= 0; i < q; ++i) {
  int a, b;
  cin >> a >> b, --a;
  mo.query(a, b);
 }
 vector<i64> ans(q);
 vector<int> cnt(xs.size(), 0);
 vector<int> st_i;
 vector<i64> st_cur;
 st_i.reserve(n + 1000), st_cur.reserve(n + 1000);
 i64 cur= 0;
 auto add= [&](int i) {
  st_i.push_back(i), st_cur.push_back(cur);
  const int v= id[i];
  ++cnt[v];
  const i64 cand= xs[v] * cnt[v];
  if(cand > cur) cur= cand;
 };
 auto undo= [&]() {
  cur= st_cur.back(), st_cur.pop_back();
  --cnt[id[st_i.back()]], st_i.pop_back();
 };
 auto out= [&](int qid) { ans[qid]= cur; };
 mo.run(add, undo, out);
 for(int i= 0; i < q; ++i) cout << ans[i] << '\n';
}
