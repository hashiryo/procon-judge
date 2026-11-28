// LOJ 2254「SNOI2017」一个简单的询问。
// 長さ N の数列と Q 個の質問 (l1,r1,l2,r2) に答える。答は Σ_x get(l1,r1,x)・get(l2,r2,x)。
// 数列の位置を 2 軸と見て、右下の部分行列 [xl,N)×[yl,N) にある等しい値の組の数を保つ
// 2 次元の Mo's algorithm (Mo2D) で解く。S(p) を [p,N) の個数とすると
// get(l,r,x)=S(l,x)-S(r,x) なので、質問は 4 つの suffix の積に展開できる。
// 行を足すときは、その行の値と同じ値を持つ列の数だけ組が増える。列を足すときも同じ。
#include <iostream>
#include <vector>
#include "neo/base/engine/query/Mo2D.hpp"
using namespace std;

signed main() {
 cin.tie(0), ios::sync_with_stdio(0);
 int n;
 cin >> n;
 vector<int> a(n);
 for(auto& x: a) cin >> x;
 int q;
 cin >> q;
 Mo2D mo;
 vector<int> qid, sign;
 qid.reserve(4 * q), sign.reserve(4 * q);
 for(int i= 0; i < q; ++i) {
  int l1, r1, l2, r2;
  cin >> l1 >> r1 >> l2 >> r2;
  // 0-indexed の半開区間 [l1, r1), [l2, r2) にする。S(p) は [p, n) の個数。
  --l1, --l2;
  const int p1= l1, p2= r1, q1= l2, q2= r2;
  // G(p,q)=Σ_x S(p,x)S(q,x) とすると、答は G(p1,q1)-G(p1,q2)-G(p2,q1)+G(p2,q2)。
  mo.query(p1, n, q1, n), qid.push_back(i), sign.push_back(1);
  mo.query(p1, n, q2, n), qid.push_back(i), sign.push_back(-1);
  mo.query(p2, n, q1, n), qid.push_back(i), sign.push_back(-1);
  mo.query(p2, n, q2, n), qid.push_back(i), sign.push_back(1);
 }
 vector<long long> ans(q, 0);
 vector<int> cu(n + 1, 0), cv(n + 1, 0);
 long long s= 0;
 auto add_x= [&](int x, int, int) {
  s+= cv[a[x]], ++cu[a[x]];
 };
 auto erase_x= [&](int x, int, int) {
  --cu[a[x]], s-= cv[a[x]];
 };
 auto add_y= [&](int y, int, int) {
  s+= cu[a[y]], ++cv[a[y]];
 };
 auto erase_y= [&](int y, int, int) {
  --cv[a[y]], s-= cu[a[y]];
 };
 auto out= [&](int id) { ans[qid[id]]+= (long long)sign[id] * s; };
 mo.run(add_x, erase_x, add_y, erase_y, out);
 for(int i= 0; i < q; ++i) cout << ans[i] << '\n';
}
