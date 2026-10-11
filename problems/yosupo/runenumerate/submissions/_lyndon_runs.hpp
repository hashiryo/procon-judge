#pragma once
// lyndon_direct と lyndon_rmq が共有する、Lyndon 根から run を集める中身 (Bannai、I、Inenaga、Nakashima、Takeda、Tsuruta 2017 の
// Runs 定理による方法)。どの run も、2 つの順序 (文字の順と、その逆) のどちらかで、最長の Lyndon 接頭辞として現れる根を持つ。
// 接尾辞配列の順位の配列 isa から、位置 i の最長の Lyndon 接頭辞の終わりを、i より後ろで順位が i より小さい最初の位置 (文字の順) と、
// 大きい最初の位置 (逆の順。文字列の終わりを、文字の順では最も小さく、逆の順では最も大きいとみなすと、逆の順の比べは順位の大小の
// 逆になる) として求める。根 [i, j) を、後ろへ LCE(i, j)、前へ LCS(i, j) だけ伸ばし、長さが周期 p = j - i の 2 倍以上なら run。
// 同じ周期の、前に見つけた run の中に収まる根は、同じ run になるので飛ばす。こうすると同じ run を 2 度見つけず、周期ごとには
// 始まりの順に見つかるので、周期で数え分けるだけで (t, l, r) の昇順に並ぶ。
// run の列挙の実装を比べた記録は algo-notes の notes/runs.md にある。
#include <array>
#include <vector>

namespace lyndon_runs {
// nxt0[i] と nxt1[i] は、i より後ろで順位が i より小さい (大きい) 最初の位置。無ければ n。
inline void lyndon_ends(const std::vector<int> &isa, std::vector<int> &nxt0, std::vector<int> &nxt1) {
  const int n = int(isa.size());
  nxt0.resize(n), nxt1.resize(n);
  std::vector<int> st;
  st.reserve(n);
  for (int i = n - 1; i >= 0; --i) {
    while (!st.empty() && isa[st.back()] > isa[i]) st.pop_back();
    nxt0[i] = st.empty() ? n : st.back();
    st.push_back(i);
  }
  st.clear();
  for (int i = n - 1; i >= 0; --i) {
    while (!st.empty() && isa[st.back()] < isa[i]) st.pop_back();
    nxt1[i] = st.empty() ? n : st.back();
    st.push_back(i);
  }
}
// 長さ n の文字列の run を (周期, 始まり, 終わり) の昇順で返す。extend(i, j, p) は根 [i, j) (周期 p) の run を {始まり, 終わり} で
// 返し、run にならないときは始まりを -1 にする。
template <class Extend> std::vector<std::array<int, 3>> collect(const std::vector<int> &isa, Extend &&extend) {
  const int n = int(isa.size());
  std::vector<std::array<int, 3>> found;
  if (n < 2) return found;
  std::vector<int> nxt0, nxt1, last_r(n + 1, 0);
  lyndon_ends(isa, nxt0, nxt1);
  auto check = [&](int i, int j) {
    const int p = j - i;
    if (i + p <= last_r[p]) return;  // 同じ周期の、前に見つけた run の中に収まる根
    const auto [l, r] = extend(i, j, p);
    if (l < 0) return;
    found.push_back({p, l, r});
    last_r[p] = r;
  };
  // 逆の順で根が文字列の終わりまで届くとき (nxt1[i] = n) は、終わりを最も大きいとみなしたせいで a^k のように原始的でない根に
  // なりうるので見ない。文字列の終わりで止まる run は、終わりを最も小さいとみなす文字の順の側で必ず見つかる。文字の順では、根が
  // 終わりまで届いても原始的になる (u^k なら u^(k-1) が後ろにあって順位が小さい)。
  for (int i = 0; i < n; ++i) {
    check(i, nxt0[i]);
    if (nxt1[i] < n) check(i, nxt1[i]);
  }
  // 周期で数え分ける (周期ごとには始まりの順に見つかっている)。
  std::vector<int> cnt(n / 2 + 2, 0);
  for (const auto &x : found) ++cnt[x[0] + 1];
  for (int p = 1; p <= n / 2 + 1; ++p) cnt[p] += cnt[p - 1];
  std::vector<std::array<int, 3>> res(found.size());
  for (const auto &x : found) res[cnt[x[0]]++] = x;
  return res;
}
}  // namespace lyndon_runs
