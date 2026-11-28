#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/Convex.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 vector<Point<int>> ps(n);
 for(auto& p: ps) cin >> p;
 Convex<int> g(ps, false);
 n= g.size();
 cout << n << '\n';
 int st= 0;
 for(int i= 0; i < n; ++i)
  if(g[st].y > g[i].y || (g[st].y == g[i].y && g[st].x > g[i].x)) st= i;
 for(int i= 0, j= st; i < n; ++i, j= g.next(j)) cout << g[j].x << " " << g[j].y << '\n';
 return 0;
}
