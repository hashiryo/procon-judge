#include <cstdio>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
using namespace std;
int main() {
 // 提出の列 [L, R) の中で、AC した問題数と、各問題の最初の AC より前の WA の総和。
 // 区間の左端に入る要素は、その問題の最初の AC になりうる (それより前の WA は無い)。
 // 右端に入る AC が最初の AC なら、いま区間にある WA が全部その前にある。
 int n, m, q;
 scanf("%d %d %d", &n, &m, &q);
 vector<int> p(n);
 vector<char> is_ac(n);
 for(int i= 0; i < n; ++i) {
  char s[4];
  scanf("%d %3s", &p[i], s), --p[i];
  is_ac[i]= (s[0] == 'A');
 }
 // ac_gap[i]: i が AC のとき、同じ問題の次の AC との間にある WA の数。
 vector<int> ac_gap(n, 0), last_ac(m, -1), wa_since(m, 0);
 for(int i= 0; i < n; ++i)
  if(is_ac[i]) {
   if(last_ac[p[i]] >= 0) ac_gap[last_ac[p[i]]]= wa_since[p[i]];
   wa_since[p[i]]= 0, last_ac[p[i]]= i;
  } else ++wa_since[p[i]];
 Mo mo;
 for(int i= 0; i < q; ++i) {
  int l, r;
  scanf("%d %d", &l, &r), --l;
  mo.query(l, r);
 }
 vector<int> cnt_ac(m, 0), cnt_wa(m, 0), pen(m, 0), ans_solved(q), ans_pen(q);
 int solved= 0, penalty= 0;
 auto add_l= [&](int i) {
  const int x= p[i];
  if(is_ac[i]) {
   if(cnt_ac[x] == 0) ++solved;
   else penalty-= ac_gap[i], pen[x]-= ac_gap[i];
   ++cnt_ac[x];
  } else {
   if(cnt_ac[x] > 0) ++pen[x], ++penalty;
   ++cnt_wa[x];
  }
 };
 auto add_r= [&](int i) {
  const int x= p[i];
  if(is_ac[i]) {
   if(cnt_ac[x] == 0) solved+= 1, penalty+= cnt_wa[x], pen[x]+= cnt_wa[x];
   ++cnt_ac[x];
  } else {
   ++cnt_wa[x];
  }
 };
 auto erase_l= [&](int i) {
  const int x= p[i];
  if(is_ac[i]) {
   if(--cnt_ac[x] == 0) --solved, pen[x]= 0;
   else penalty+= ac_gap[i], pen[x]+= ac_gap[i];
  } else {
   if(cnt_ac[x] > 0) --pen[x], --penalty;
   --cnt_wa[x];
  }
 };
 auto erase_r= [&](int i) {
  const int x= p[i];
  if(is_ac[i]) {
   if(--cnt_ac[x] == 0) --solved, penalty-= pen[x], pen[x]= 0;
  } else {
   --cnt_wa[x];
  }
 };
 auto out= [&](int id) { ans_solved[id]= solved, ans_pen[id]= penalty; };
 mo.run(add_l, add_r, erase_l, erase_r, out);
 for(int i= 0; i < q; ++i) printf("%d %d\n", ans_solved[i], ans_pen[i]);
 return 0;
}
