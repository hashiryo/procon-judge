// _shared/flow/proto_assignment.hpp の試作の API で書いたもの。長さの和が最小の完全マッチングは交差しないので、距離を
// double の行列にして assignment に渡し、行ごとの列をそのまま出す。
#include <cmath>
#include <cstdio>
#include <vector>
#include "_shared/flow/proto_assignment.hpp"
int main() {
 int n;
 if(scanf("%d", &n) != 1) return 1;
 std::vector<long long> x(2 * n), y(2 * n);
 for(int i= 0; i < 2 * n; ++i)
  if(scanf("%lld %lld", &x[i], &y[i]) != 2) return 1;
 std::vector<double> d((size_t)n * n);
 for(int i= 0; i < n; ++i)
  for(int j= 0; j < n; ++j) {
   const long long dx= x[i] - x[n + j], dy= y[i] - y[n + j];
   d[(size_t)i * n + j]= std::sqrt(double(dx * dx + dy * dy));
  }
 const auto r= proto::assignment(n, n, d);
 for(int i= 0; i < n; ++i) printf("%d%c", r.col[i] + 1, i + 1 == n ? '\n' : ' ');
}
