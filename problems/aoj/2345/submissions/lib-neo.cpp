#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "neo/algebra/ZMod.hpp"
#include "neo/set_power_series/graph.hpp"
using namespace std;
using Z= ZMod<998244353>;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 cout << fixed << setprecision(12);
 int N, M, P;
 cin >> N >> M >> P;
 sps::Edges es;
 for(int i= 0, u, v; i < M; i++) cin >> u >> v, es.push_back({u - 1, v - 1});
 if(P == 0) return cout << (sps::connected_component_num(N, es).back() == 1) << '\n', 0;
 auto e= sps::edge_num(N, es);
 vector<double> h(e.size());
 for(int s= e.size(); s--;) h[s]= pow(100. / P, e[s]);
 auto f= sps::log(h);
 cout << f.back() * pow(double(P) / 100, e.back()) << '\n';
 return 0;
}
