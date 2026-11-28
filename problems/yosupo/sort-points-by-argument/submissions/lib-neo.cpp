#include <algorithm>
#include <iostream>
#include <vector>
#include "neo/base/core/geo/angle.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 int N;
 cin >> N;
 vector<Point<int>> ps(N);
 for(int i= 0; i < N; ++i) cin >> ps[i];
 sort(ps.begin(), ps.end(), AngleComp<int>());
 for(auto& p: ps) cout << p.x << " " << p.y << "\n";
 return 0;
}
