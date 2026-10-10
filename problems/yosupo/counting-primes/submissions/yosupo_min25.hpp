#pragma once
// Library Checker の表を持たない C++ の上位 (12〜14 ms) が使う、min_25 の素数の個数。
// 写したのは sortA0329 の提出 222872 (https://judge.yosupo.jp/submission/222872) の CountPrimes で、
// lpha の 195894 や rich_brain の 53928 も同じ形。入出力の部分は除いた。
//
// Lucy の DP を奇数だけで持ち、篩い終えた数 (roughs) を詰めて残りの添字だけを回す。
// 大きい側の商 N / r は double で 1 回求めて invs に持ち、素数 p での割り算は
// 2^64 / p の逆数との掛け算で置き換える。√√N より大きい素数の寄与は最後にまとめて足す。
// larges は元と同じく u32 で持つので、途中の値は 2^32 を法として合っている。答えが 2^32 未満
// (N ≤ 10^11 なら 4118054813 以下) なので、最後に u32 へ落とせば正しい。
#include "../common.hpp"

inline u64 run(u64 N) {
    if (N <= 1) return 0;
    u64 v = (u64)std::sqrt((double)N);
    while (v * v > N) --v;
    while ((v + 1) * (v + 1) <= N) ++v;
    u32 s = (u32)((v + 1) / 2);
    std::vector<u64> invs(s);
    std::vector<u32> smalls(s), larges(s), roughs(s);
    std::vector<char> smooth(v + 2, 0);
    for (u32 i = 0; i != s; ++i) smalls[i] = i;
    for (u32 i = 0; i != s; ++i) roughs[i] = 2 * i + 1;
    for (u32 i = 0; i != s; ++i) invs[i] = (u64)((double)N / roughs[i]);
    for (u32 i = 0; i != s; ++i) larges[i] = (u32)((invs[i] - 1) / 2);
    u32 pc = 0;
    for (u64 p = 3; p * p <= v; p += 2) {
        if (smooth[p]) continue;
        for (u64 i = p * p; i <= v; i += 2 * p) smooth[i] = 1;
        smooth[p] = 1;
        const u64 invp = ~u64(0) / p + 1;
        auto divide_p = [invp](u64 inv_j) -> u64 { return (u64)(((u128)inv_j * invp) >> 64); };
        u32 ns = 0;
        u32 k = 0;
        for (;; ++k) {
            const u32 j = roughs[k];
            if ((u64)j * p > v) break;
            if (smooth[j]) continue;
            larges[ns] = larges[k] - larges[smalls[j * p / 2] - pc] + pc;
            invs[ns] = invs[k];
            roughs[ns] = roughs[k];
            ++ns;
        }
        for (; k < s; ++k) {
            const u32 j = roughs[k];
            if (smooth[j]) continue;
            larges[ns] = larges[k] - smalls[(divide_p(invs[k]) - 1) / 2] + pc;
            invs[ns] = invs[k];
            roughs[ns] = roughs[k];
            ++ns;
        }
        s = ns;
        u64 i = (v - 1) / 2;
        for (u64 j = (divide_p(v) - 1) | 1; j >= p; j -= 2) {
            const u32 d = smalls[j / 2] - pc;
            for (; i >= j * p / 2; --i) smalls[i] -= d;
        }
        ++pc;
    }
    u64 ret = 1;
    ret += (u64)larges[0] + (u64)s * (s - 1) / 2 + (u64)(pc - 1) * (s - 1);
    for (u32 k = 1; k < s; ++k) ret -= larges[k];
    for (u32 k1 = 1; k1 < s; ++k1) {
        const u64 p = roughs[k1];
        const u64 invp = ~u64(0) / p + 1;
        auto divide_p = [invp](u64 inv_j) -> u64 { return (u64)(((u128)inv_j * invp) >> 64); };
        const u32 k2_max = smalls[(divide_p(invs[k1]) - 1) / 2] - pc;
        if (k2_max <= k1) break;
        for (u32 k2 = k1 + 1; k2 <= k2_max; ++k2) ret += smalls[(divide_p(invs[k2]) - 1) / 2];
        ret -= (u64)(k2_max - k1) * (pc + k1 - 1);
    }
    return (u32)ret;
}
