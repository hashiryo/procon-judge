#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>
#include "mylib/geometry/Point.hpp"
using namespace std;
struct UnionFind {
 vector<int> p;
 UnionFind(int n): p(n, -1) {}
 int find(int x) { return p[x] < 0 ? x : p[x]= find(p[x]); }
 bool same(int a, int b) { return find(a) == find(b); }
 void unite(int a, int b) {
  a= find(a), b= find(b);
  if(a == b) return;
  if(p[a] > p[b]) swap(a, b);
  p[a]+= p[b], p[b]= a;
 }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using namespace geo;
 int n;
 cin >> n;
 vector<Point<long long>> ps(n);
 for(auto& p: ps) cin >> p;
 vector<pair<int, int>> edges;
 vector<int> ids(n);
 iota(ids.begin(), ids.end(), 0);
 const long long NEG= numeric_limits<long long>::min();
 for(int ph= 0; ph < 4; ++ph) {
  sort(ids.begin(), ids.end(), [&](int i, int j) {
   long long a= ps[i].x + ps[i].y, b= ps[j].x + ps[j].y;
   return a != b ? a > b : ps[i].y > ps[j].y;
  });
  vector<long long> xs;
  xs.reserve(n);
  for(auto& p: ps) xs.push_back(p.x);
  sort(xs.begin(), xs.end());
  xs.erase(unique(xs.begin(), xs.end()), xs.end());
  int m= xs.size();
  vector<pair<long long, int>> fen(m, pair<long long, int>{NEG, -1});
  for(int id: ids) {
   int xi= lower_bound(xs.begin(), xs.end(), ps[id].x) - xs.begin();
   pair<long long, int> ma{NEG, -1};
   for(int i= xi + 1; i > 0; i-= i & -i)
    if(ma.first <= fen[i - 1].first) ma= fen[i - 1];
   if(ma.second != -1) edges.emplace_back(id, ma.second);
   long long v= ps[id].x - ps[id].y;
   for(int i= xi + 1; i <= m; i+= i & -i)
    if(fen[i - 1].first <= v) fen[i - 1]= {v, id};
  }
  for(auto& p: ps) swap(p.x, p.y);
  if(ph == 1)
   for(auto& p: ps) p.y= -p.y;
 }
 auto md= [&](int i, int j) { return llabs(ps[i].x - ps[j].x) + llabs(ps[i].y - ps[j].y); };
 sort(edges.begin(), edges.end(), [&](auto e, auto f) { return md(e.first, e.second) < md(f.first, f.second); });
 UnionFind uf(n);
 long long sum= 0;
 vector<pair<int, int>> res;
 for(auto [u, v]: edges)
  if(!uf.same(u, v)) uf.unite(u, v), sum+= md(u, v), res.emplace_back(u, v);
 cout << sum << '\n';
 for(auto [u, v]: res) cout << u << ' ' << v << '\n';
 return 0;
}
