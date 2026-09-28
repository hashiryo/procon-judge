// abc150-f の愚直解。k ごとに x = a_k xor b_0 と決め、全部の i で a_{(i+k) mod N} xor x = b_i かを確かめる。
// ハッシュを使わないので、Nimber の RollingHash とは別の考え方になる。O(N^2) なので、小さい入力でだけ使う。
#include <cstdio>
#include <vector>

int main() {
  int n;
  if (scanf("%d", &n) != 1) return 1;
  std::vector<unsigned> a(n), b(n);
  for (auto &v : a)
    if (scanf("%u", &v) != 1) return 1;
  for (auto &v : b)
    if (scanf("%u", &v) != 1) return 1;
  for (int k = 0; k < n; ++k) {
    unsigned x = a[k] ^ b[0];
    bool ok = true;
    for (int i = 0; i < n && ok; ++i) ok = (a[(i + k) % n] ^ x) == b[i];
    if (ok) printf("%d %u\n", k, x);
  }
}
