#include <iostream>
#include "mylib/algebra/Nimber.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 Nimber::init();
 int T;
 cin >> T;
 for(int i= 0; i < T; ++i) {
  Nimber a, b;
  cin >> a >> b;
  cout << a * b << '\n';
 }
 return 0;
}
