#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "neo/base/engine/query/Mo.hpp"
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

signed main() {
  const int n = read_int(), q = read_int();
  vector<int> a(n);
  for (auto& x : a) x = read_int();
  vector<int> vals = a;
  sort(vals.begin(), vals.end()), vals.erase(unique(vals.begin(), vals.end()), vals.end());
  for (auto& x : a) x = (int)(lower_bound(vals.begin(), vals.end(), x) - vals.begin());
  Mo mo;
  for (int i = 0; i < q; ++i) {
    const int l = read_int(), r = read_int();
    mo.query(l - 1, r);
  }
  vector<int> cnt(vals.size(), 0), ans(q);
  int distinct = 0;
  auto add = [&](int i) {
    if (cnt[a[i]]++ == 0) ++distinct;
  };
  auto erase = [&](int i) {
    if (--cnt[a[i]] == 0) --distinct;
  };
  auto out = [&](int id) { ans[id] = distinct; };
  mo.run(add, erase, out);
  string s;
  s.reserve((size_t)q * 8);
  for (int i = 0; i < q; ++i) s += to_string(ans[i]), s += '\n';
  fwrite(s.data(), 1, s.size(), stdout);
}
