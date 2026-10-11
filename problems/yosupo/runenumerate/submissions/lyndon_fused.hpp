#pragma once
// lyndon_fs_hybrid2 で、2 つの Lyndon 配列を持たずに、FastSet で根の終わりを求めるループの中で根をすぐ調べる版。根を位置の順でなく
// 接尾辞配列の順に調べるので、同じ周期の前に見つけた run の中の根を飛ばす手 (周期ごとの最後の run の終わり) は使えない。代わりに、
// 根 [i, j) の手前の一致 LCS(i, j) が周期 p 以上なら飛ばす (Ellert と Fischer の Algorithm 1 と同じ)。そのときは根 [i - p, i) も同じ
// run の根で、run は手前へ周期の分だけずらした根から見つかる。手前の一致を先に p まで調べ、p に届かない根だけ後ろへ伸ばすので、
// run の中の根は後ろへ伸ばさない。根は 8 バイトずつ比べ (lyndon_ll_w8 と同じ)、ループで比べたバイトの数が全体で 32n を超えたら LCE と
// LCS を LCP 配列と RangeMin で引く形に切り替える (最悪でも O(n))。見つかる順はでたらめなので、(周期, 始まり) を 12 bit ずつの基数
// ソートで並べる。確保する配列は、接尾辞配列、FastSet 2 つ (合わせて n / 4 バイトほど)、文字列の写し、run の列だけになる。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "pj.hpp"
#include "neo/string/suffix_array.hpp"
#include "neo/data_structure/FastSet.hpp"
#include "_lyndon_hybrid.hpp"

namespace lyndon_fused {
inline uint64_t load(const unsigned char *p) {
  uint64_t x;
  memcpy(&x, p, 8);
  return x;
}
// t[i + k] = t[j + k] が続く長さ。e 文字までは一致していると分かっているときに、lim 文字を上限として求める。
inline int forward(const unsigned char *t, int i, int j, int e, int lim) {
  for (; e < lim; e += 8)
    if (const uint64_t x = load(t + i + e) ^ load(t + j + e)) return min(e + (countr_zero(x) >> 3), lim);
  return lim;
}
// t[i - 1 - k] = t[j - 1 - k] が続く長さ。b 文字までは一致していると分かっているときに、lim 文字を上限とし、stop 文字以上と分かった
// ところで止める (止めたときの値は下限)。
inline int backward(const unsigned char *t, int i, int j, int b, int lim, int stop) {
  for (; b < lim && b < stop; b += 8)
    if (const uint64_t y = load(t + i - 8 - b) ^ load(t + j - 8 - b)) return min(b + (countl_zero(y) >> 3), lim);
  return min(b, lim);
}
// 予算を使い切ったあとの道。初めて呼ばれたときに LCE の表を作り、根 [i, j) の {手前の一致, 後ろの一致} を返す。
struct Slow {
  const string &s;
  const vector<int> &sa;
  unique_ptr<lyndon_hybrid::Lce> fwd, bwd;
  [[gnu::noinline]] pair<int, int> operator()(int i, int j) {
    const int n = int(s.size());
    if (!fwd) {
      const string r(s.rbegin(), s.rend());
      fwd = make_unique<lyndon_hybrid::Lce>(s, sa), bwd = make_unique<lyndon_hybrid::Lce>(r, suffix_array(r));
    }
    return {i > 0 ? (*bwd)(n - i, n - j) : 0, j < n ? (*fwd)(i, j) : 0};  // 反転した文字列で、n - i と n - j から始まる接尾辞
  }
};
// (周期, 始まり) の昇順に並べる。周期は n / 2 以下、始まりは n 未満。
inline void sort_runs(vector<array<int, 3>> &a, int n) {
  const int lb = bit_width(unsigned(n)), bits = lb + bit_width(unsigned(n / 2));
  vector<array<int, 3>> b(a.size());
  for (int sh = 0; sh < bits; sh += 12) {
    array<int, 4097> cnt{};
    auto digit = [&](const array<int, 3> &x) { return int(((uint64_t(x[0]) << lb | unsigned(x[1])) >> sh) & 4095); };
    for (const auto &x : a) ++cnt[digit(x) + 1];
    for (int d = 0; d < 4096; ++d) cnt[d + 1] += cnt[d];
    for (const auto &x : a) b[cnt[digit(x)]++] = x;
    a.swap(b);
  }
}
}  // namespace lyndon_fused

struct Solver {
  string s;
  vector<array<int, 3>> ans;

  explicit Solver(const string &s) : s(s) {}

  void run() {
    using namespace lyndon_fused;
    const int n = int(s.size());
    if (n < 2) return;
    const vector<int> sa = suffix_array(s);
    vector<unsigned char> buf(n + 16, 0);
    memcpy(buf.data() + 8, s.data(), n);
    const unsigned char *t = buf.data() + 8;
    long long budget = 32LL * n;
    Slow slow{s, sa, nullptr, nullptr};
    vector<array<int, 3>> found;
    auto check = [&](int i, int j) __attribute__((always_inline)) {
      const int p = j - i;
      if (2 * p > n) return;  // 長さが 2p に届かない
      if (__builtin_expect(budget < 0, 0)) {
        const auto [b, e] = slow(i, j);
        if (b < p && e + b >= p) found.push_back({p, i - b, j + e});
        return;
      }
      // 根の手前の 8 バイトと後ろの 8 バイト
      int e = countr_zero(load(t + i) ^ load(t + j)) >> 3, b = countl_zero(load(t + i - 8) ^ load(t + j - 8)) >> 3;
      if (__builtin_expect(e < 8 && b < 8, 1)) {
        e = min(e, n - j), b = min(b, i);
        if (b < p && e + b >= p) found.push_back({p, i - b, j + e});
        return;
      }
      b = b < 8 ? min(b, i) : backward(t, i, j, 8, i, p);
      budget -= b;
      if (b >= p) return;  // run の中の根。run は手前へずらした根から見つかる
      e = e < 8 ? min(e, n - j) : forward(t, i, j, 8, n - j);
      budget -= e;
      if (e + b >= p) found.push_back({p, i - b, j + e});
    };
    {
      FastSet smaller(n), larger(n, [](int) { return true; });
      for (int k = 0; k < n; ++k) {
        const int i = sa[k];
        larger.erase(i);
        const int a = smaller.next(i + 1), c = larger.next(i + 1);
        smaller.insert(i);
        check(i, a < 0 ? n : a);
        if (c >= 0) check(i, c);  // 逆の順で根が文字列の終わりまで届く候補は見ない (_lyndon_runs.hpp の collect と同じ)
      }
    }
    sort_runs(found, n);
    ans = std::move(found);
  }

  const vector<array<int, 3>> &answer() const { return ans; }
};
