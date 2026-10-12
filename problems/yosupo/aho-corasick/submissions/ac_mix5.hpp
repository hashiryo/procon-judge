#pragma once
// 入力の形で ac_goto3 と ac_hybrid5 を選ぶ版。3 回目 (7a0760d5 の run 38215267734、Xeon 8573C) の x64-gcc のケースごとの時間では、
// 周期的な文字列やフィボナッチ文字列でできるほぼ 1 本道のトライと、26 文字の入力ではどれも ac_hybrid3 が速く (fibonacci_15 で
// 10.85 ms と 17.74 ms)、2 文字から 5 文字のでたらめな文字列を 3 本以上並べた入力では ac_goto3 が速かった (random_03 で 14.55 ms と
// 30.44 ms)。文字列が 1 本か 2 本なら、でたらめでも ac_hybrid3 が並ぶか速い。周期的かどうかは、先頭から最大 65536 個の長さ 12 の
// 部分文字列 (文字列をまたがない) の種類の数で見る。ケースでは、周期的な入力は 100 種類以下、でたらめな入力は 4096 種類以上だった。
// ここでは 1 文字 3 bit に詰めた 36 bit の値を 2^16 bit の表に散らし、立った bit が 1024 以下なら周期的とみる。
// Aho–Corasick の実装を比べた記録は algo-notes の notes/aho-corasick.md にある。
#include <cstdint>
#include <string>
#include <vector>
#include "pj.hpp"
#include "_ac_goto3.hpp"
#include "_ac_hybrid5.hpp"

namespace ac_mix5 {
// 長さ 12 の部分文字列の種類が少ないか (k <= 8 のとき)。長さ 12 以上の文字列が無ければ false。
inline bool repetitive(const std::vector<std::string> &ss, const ac_common::Coded &cd) {
  std::vector<uint64_t> bits(1 << 10, 0);
  constexpr uint64_t MASK = (uint64_t(1) << 36) - 1;
  int seen = 0;
  for (const auto &s : ss) {
    if (s.size() < 12) continue;
    uint64_t x = 0;
    for (size_t i = 0; i < s.size() && seen < 65536; ++i) {
      x = (x << 3 | cd.code[(unsigned char)s[i]]) & MASK;
      if (i >= 11) {
        const uint64_t h = (x * 0x9E3779B97F4A7C15ull) >> 48;
        bits[h >> 6] |= uint64_t(1) << (h & 63), ++seen;
      }
    }
    if (seen >= 65536) break;
  }
  if (!seen) return false;
  int distinct = 0;
  for (const uint64_t b : bits) distinct += __builtin_popcountll(b);
  return distinct <= 1024;
}
}  // namespace ac_mix5

struct Solver {
  vector<string> ss;
  AhoCorasickAnswer ans;

  explicit Solver(const vector<string> &ss) : ss(ss) {}

  void run() {
    const ac_common::Coded cd = ac_common::remap_total(ss);
    if (cd.k > 8 || ss.size() <= 2 || ac_mix5::repetitive(ss, cd)) ac_hybrid5_body::solve(ss, cd, ans);
    else ac_goto3_body::solve(ss, cd, ans);
  }

  const AhoCorasickAnswer &answer() const { return ans; }
};
