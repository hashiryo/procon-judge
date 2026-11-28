// 期待出力を作る参照実装。区間 distinct を、右端でソートして各値の最後の出現位置を BIT に持つ
// オフラインの解法で解く。クエリ [l, r] の答えは、r までで最後の出現位置が l 以上にある値の数。
// ライブラリは include しない。愚直解 (brute.cpp) と小さい入力で突き合わせる。
#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
using namespace std;

namespace {
char buf[1 << 20];
int len = 0, pos = 0;

int get_char() {
  if (pos == len) {
    len = (int)fread(buf, 1, sizeof(buf), stdin), pos = 0;
    if (len <= 0) return -1;
  }
  return buf[pos++];
}

int read_int() {
  int c = get_char();
  while (c >= 0 && c <= 32) c = get_char();
  int x = 0;
  while (c > 32) x = x * 10 + (c - '0'), c = get_char();
  return x;
}
}  // namespace

int main() {
  const int n = read_int(), q = read_int();
  vector<int> a(n + 1);
  vector<int> vals;
  vals.reserve(n);
  for (int i = 1; i <= n; ++i) a[i] = read_int(), vals.push_back(a[i]);
  sort(vals.begin(), vals.end()), vals.erase(unique(vals.begin(), vals.end()), vals.end());
  vector<int> comp(n + 1);
  for (int i = 1; i <= n; ++i) comp[i] = (int)(lower_bound(vals.begin(), vals.end(), a[i]) - vals.begin());

  vector<int> ql(q), qr(q), order(q);
  for (int i = 0; i < q; ++i) ql[i] = read_int(), qr[i] = read_int();
  for (int i = 0; i < q; ++i) order[i] = i;
  sort(order.begin(), order.end(), [&](int x, int y) { return qr[x] < qr[y]; });

  vector<int> bit(n + 1, 0), last(vals.size(), 0), ans(q);
  auto add = [&](int i, int x) {
    for (; i <= n; i += i & -i) bit[i] += x;
  };
  auto sum = [&](int i) {
    int s = 0;
    for (; i > 0; i -= i & -i) s += bit[i];
    return s;
  };
  int qi = 0;
  for (int i = 1; i <= n; ++i) {
    const int v = comp[i];
    if (last[v]) add(last[v], -1);
    add(i, 1), last[v] = i;
    while (qi < q && qr[order[qi]] == i) {
      const int id = order[qi++];
      ans[id] = sum(i) - sum(ql[id] - 1);
    }
  }
  string out;
  out.reserve((size_t)q * 8);
  for (int i = 0; i < q; ++i) out += to_string(ans[i]), out += '\n';
  fwrite(out.data(), 1, out.size(), stdout);
  return 0;
}
