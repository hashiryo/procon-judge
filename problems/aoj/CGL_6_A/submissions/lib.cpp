#include <algorithm>
#include <iostream>
#include <vector>
#include "mylib/data_structure/BinaryIndexedTree.hpp"
#include "mylib/geometry/Segment.hpp"
using namespace std;
// 軸平行な線分の集合で、縦と横が交わる組の数を数える。x の小さい順に横の線分の y を足し引きし、
// 縦の線分の位置で y の範囲の和を取る。
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 using K= long long;
 struct Ev {
  K x;
  int type;
  K a, b;
 };
 int n;
 cin >> n;
 vector<Ev> ev;
 vector<K> ys;
 ev.reserve(n * 2), ys.reserve(n);
 for(int i= 0; i < n; ++i) {
  Point<K> p, q;
  cin >> p >> q;
  if(p.y == q.y) {
   K l= min(p.x, q.x), r= max(p.x, q.x);
   ev.push_back({l, 0, p.y, p.y}), ev.push_back({r, 2, p.y, p.y});
   ys.push_back(p.y);
  } else {
   K l= min(p.y, q.y), r= max(p.y, q.y);
   ev.push_back({p.x, 1, l, r});
  }
 }
 sort(ev.begin(), ev.end(), [](const Ev& a, const Ev& b) { return a.x != b.x ? a.x < b.x : a.type < b.type; });
 sort(ys.begin(), ys.end()), ys.erase(unique(ys.begin(), ys.end()), ys.end());
 BinaryIndexedTree<int> fw(ys.size());
 long long ans= 0;
 for(const auto& e: ev) {
  if(e.type == 0) fw.add(lower_bound(ys.begin(), ys.end(), e.a) - ys.begin(), 1);
  else if(e.type == 2) fw.add(lower_bound(ys.begin(), ys.end(), e.a) - ys.begin(), -1);
  else ans+= fw.sum(lower_bound(ys.begin(), ys.end(), e.a) - ys.begin(), upper_bound(ys.begin(), ys.end(), e.b) - ys.begin());
 }
 cout << ans << '\n';
 return 0;
}
