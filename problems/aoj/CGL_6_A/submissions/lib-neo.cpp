#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/count_axis_parallel_intersections.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 int n;
 cin >> n;
 vector<Segment<long long>> ss(n);
 for(auto& s: ss) cin >> s.p >> s.q;
 cout << count_axis_parallel_intersections(ss) << '\n';
 return 0;
}
