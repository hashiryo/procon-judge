// 自作の問題。GF(2) の線形マトロイド (64 bit のベクトルの xor での線形独立) の和を、性質の違う族で測る。ベクトルに
// 組の番号を割り当て (使わないベクトルは -1)、同じ番号のベクトルが線形独立になるようにして、割り当てたベクトルの数を
// 最大にする。k = 2 は AtCoder game_x (XOR Bag) の形。
//
// 提出は次を実装する。
//   struct Solver {
//     Solver(const vector<u64> &vec, int k);
//     void run();                  // 割り当てを求める
//     vector<int> answer() const;  // ベクトルごとの組の番号 (0 以上 k 未満、使わないベクトルは -1)
//   };
// k >= 1。0 のベクトルもありうる。
//
// 入力は 1 行で、族の名前、seed、引数を並べる。ベクトルはハーネスが計測の前に作り、順を乱択で並べ替える。構築と run を
// 測る。answer は計測区間の外で呼び、ハーネスが長さと番号の範囲、組ごとに線形独立であることを確かめてから、割り当てた
// ベクトルの数を出す (崩れていれば invalid)。引数の参照は計測区間のあとまで生きているので、提出は参照を持ってよい。
//
//   random M D K      下位 D bit が乱択のベクトルを M 本
//   low M D R K       D bit の乱択の R 本が張る空間から乱択に選んだベクトルを M 本 (どの組も R 本まで)
//   graph M D K       e_u xor e_v (D 個の bit から u != v を乱択) の形のベクトルを M 本。D 頂点のグラフのグラフ的マトロイドと同じ
//   dup M D R K       下位 D bit が乱択の R 本のどれかを写したベクトルを M 本 (同じベクトルが多い)
//   sparse M D W K    D bit のうち乱択の W bit を立てたベクトルを M 本
#include "pj.hpp"
#include <algorithm>
#include <string>

#ifndef SUBMISSION_HPP
#define SUBMISSION_HPP "reference.hpp"
#endif
#include SUBMISSION_HPP

namespace gen {
struct Rng {
  u64 x;
  u64 next() {
    u64 z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
  }
  // [0, n)
  int below(u64 n) { return (int)((unsigned __int128)next() * n >> 64); }
  u64 bits(int d) { return d >= 64 ? next() : next() & ((1ULL << d) - 1); }
};

vector<u64> random(Rng &rng, int M, int D) {
  vector<u64> v(M);
  for (auto &x : v) x = rng.bits(D);
  return v;
}

vector<u64> low(Rng &rng, int M, int D, int R) {
  vector<u64> base(R), v(M);
  for (auto &b : base) b = rng.bits(D);
  for (auto &x : v) {
    const u64 s = rng.bits(R);
    x = 0;
    for (int k = 0; k < R; ++k)
      if (s >> k & 1) x ^= base[k];
  }
  return v;
}

vector<u64> graph(Rng &rng, int M, int D) {
  vector<u64> v(M);
  for (auto &x : v) {
    const int u = rng.below(D);
    int w = rng.below(D - 1);
    if (w >= u) ++w;
    x = (1ULL << u) | (1ULL << w);
  }
  return v;
}

vector<u64> dup(Rng &rng, int M, int D, int R) {
  vector<u64> base(R), v(M);
  for (auto &b : base) b = rng.bits(D);
  for (auto &x : v) x = base[rng.below(R)];
  return v;
}

vector<u64> sparse(Rng &rng, int M, int D, int W) {
  vector<u64> v(M);
  for (auto &x : v) {
    x = 0;
    for (int k = 0; k < W; ++k) x |= 1ULL << rng.below(D);
  }
  return v;
}
}  // namespace gen

signed main() {
  const string fam = read_token();
  long long seed;
  must_scan(scanf("%lld", &seed), 1);
  gen::Rng rng{(u64)seed};
  auto args = [&](int k) {
    vector<int> a(k);
    for (auto &x : a) must_scan(scanf("%d", &x), 1);
    return a;
  };
  vector<u64> v;
  int k = 1;
  if (fam == "random") { auto a = args(3); v = gen::random(rng, a[0], a[1]), k = a[2]; }
  else if (fam == "low") { auto a = args(4); v = gen::low(rng, a[0], a[1], a[2]), k = a[3]; }
  else if (fam == "graph") { auto a = args(3); v = gen::graph(rng, a[0], a[1]), k = a[2]; }
  else if (fam == "dup") { auto a = args(4); v = gen::dup(rng, a[0], a[1], a[2]), k = a[3]; }
  else if (fam == "sparse") { auto a = args(4); v = gen::sparse(rng, a[0], a[1], a[2]), k = a[3]; }
  else {
    fprintf(stderr, "unknown family %s\n", fam.c_str());
    return 1;
  }
  for (size_t i = v.size(); i > 1; --i) swap(v[i - 1], v[rng.below(i)]);

  auto t0 = chrono::steady_clock::now();
  Solver sol(v, k);
  sol.run();
  auto t1 = chrono::steady_clock::now();

  const vector<int> ans = sol.answer();
  const int m = v.size();
  string bad;
  int cnt = 0;
  if ((int)ans.size() != m) bad = "answer length";
  else {
    vector<array<u64, 64>> basis(k);
    for (auto &b : basis) b.fill(0);
    for (int i = 0; i < m && bad.empty(); ++i) {
      if (ans[i] < -1 || ans[i] >= k) { bad = "group id out of range"; break; }
      if (ans[i] < 0) continue;
      auto &b = basis[ans[i]];
      u64 x = v[i];
      while (x && b[63 - __builtin_clzll(x)]) x ^= b[63 - __builtin_clzll(x)];
      if (!x) { bad = "linearly dependent"; break; }
      b[63 - __builtin_clzll(x)] = x, ++cnt;
    }
  }
  if (bad.empty()) printf("%d\n", cnt);
  else printf("invalid: %s\n", bad.c_str());

  report_metrics((long long)chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count());
}
