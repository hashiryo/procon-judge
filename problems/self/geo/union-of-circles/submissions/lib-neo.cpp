#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algo/compute/geo/union_of_circles_area.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 using R= long double;
 int N;
 cin >> N;
 vector<Circle<R>> cs(N);
 for(auto& c: cs) cin >> c.o >> c.r;
 cout << fixed << setprecision(10) << union_of_circles_area(cs) << '\n';
 return 0;
}
