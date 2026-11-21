// https://atcoder.jp/contests/arc033/tasks/arc033_4
// NeoLibrary の ZMod と interpolate_iota (neo/algo/compute/poly/lagrange.hpp)。
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include "neo/base/core/algebra/ZMod.hpp"
#include "neo/algo/compute/poly/lagrange.hpp"
using namespace std;
bool test(int (*solve)(stringstream&, stringstream&), string in, string expected) {
 stringstream scin(in), scout;
 solve(scin, scout);
 return scout.str() == expected;
}
namespace TEST {
signed main(stringstream& scin, stringstream& scout) {
 using Mint= ZMod<int(1e9 + 7)>;
 int N;
 scin >> N;
 vector<Mint> A(N + 1);
 for(int i= 0; i <= N; ++i) scin >> A[i];
 int T;
 scin >> T;
 scout << interpolate_iota<Mint>(A, T) << '\n';
 return 0;
}
}
signed main() {
 assert(test(TEST::main, "2\n1 3 7\n3\n", "13\n"));
 assert(test(TEST::main, "5\n4 16 106 484 1624 4384\n1000000000\n", "999984471\n"));
 return 0;
}
