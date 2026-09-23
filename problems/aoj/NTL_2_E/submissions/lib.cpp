#include <iostream>
#include "mylib/fft/BigInt.hpp"
using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(false);
 BigInt A, B;
 cin >> A >> B;
 cout << A % B << '\n';
 return 0;
}
