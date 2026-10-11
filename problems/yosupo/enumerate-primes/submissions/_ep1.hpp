#pragma once
// 素数の列挙の 1 回目。素数の個数 (NeoLibrary 7b624cb の prime_pi) の篩を列挙の形にしたもの。30 の車輪のビット列を
// 区画 (SW 語) ごとに作る。7、11、13 は 1001 バイト周期の型を写し、17 と 19、23 と 29、31 と 37 は 2 つずつまとめた
// 周期の型を AND し、41 以上 √N 以下の素数は倍数の車輪の 1 周 8 個を並べて p^2 から消す。区画ごとに popcount で数え、
// B 番目から A 個おきの素数は、語ごとの個数を見て、語の中の k 番目のビットを pdep で引いて取り出す。
#include "../common.hpp"
#if defined(__x86_64__) && defined(__BMI2__)
#include <immintrin.h>
#endif

namespace ep1 {

inline constexpr u32 WR[8] = {1, 7, 11, 13, 17, 19, 23, 29};
inline constexpr std::array<u8, 30> BI = [] {
    std::array<u8, 30> t{};
    for (auto& v : t) v = 255;
    for (u32 i = 0; i < 8; ++i) t[WR[i]] = (u8)i;
    return t;
}();
// p ≡ WR[c] (mod 30) の倍数 p m (m ≡ WR[i]) のビットの位置と、m が 30 進む 1 周の先頭からのバイトの差の端数
inline constexpr std::array<std::array<u8, 8>, 8> WBIT = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 i = 0; i < 8; ++i) t[c][i] = BI[WR[c] * WR[i] % 30];
    return t;
}();
inline constexpr std::array<std::array<u8, 8>, 8> WOFF = [] {
    std::array<std::array<u8, 8>, 8> t{};
    for (u32 c = 0; c < 8; ++c)
        for (u32 i = 0; i < 8; ++i) t[c][i] = (u8)(WR[c] * WR[i] / 30);
    return t;
}();

// 篩う素数の状態。pos は今の区画の先頭からのバイトの位置、i は倍数 p m の m の車輪の位置。
struct SievingPrime {
    u32 pos, a;
    u8 cls, i;
};

// 区画の [0, nbytes) バイトにある p の倍数を消す。周の途中は 1 つずつ、周が丸ごと入るところは 8 個を並べて消す。
template <u32 C>
inline void cross_cls(u8* seg, u32 nbytes, SievingPrime& s) {
    const u32 a = s.a, p = 30 * a + WR[C];
    const u32 off[8] = {0, a * 6 + WOFF[C][1], a * 10 + WOFF[C][2], a * 12 + WOFF[C][3],
                        a * 16 + WOFF[C][4], a * 18 + WOFF[C][5], a * 22 + WOFF[C][6], a * 28 + WOFF[C][7]};
    auto clr = [&](u32 q, u32 bit) { seg[q] &= (u8)~(1u << bit); };
    u32 i = s.i, base = s.pos - off[i];  // 周の先頭 (区画より前なら 2^32 で回る)
    for (; i < 8; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) {
            s.pos = q - nbytes, s.i = (u8)i;
            return;
        }
        clr(q, WBIT[C][i]);
    }
    for (base += p; base + off[7] < nbytes; base += p) {
        clr(base, WBIT[C][0]), clr(base + off[1], WBIT[C][1]), clr(base + off[2], WBIT[C][2]), clr(base + off[3], WBIT[C][3]);
        clr(base + off[4], WBIT[C][4]), clr(base + off[5], WBIT[C][5]), clr(base + off[6], WBIT[C][6]), clr(base + off[7], WBIT[C][7]);
    }
    for (i = 0;; ++i) {
        const u32 q = base + off[i];
        if (q >= nbytes) {
            s.pos = q - nbytes, s.i = (u8)i;
            return;
        }
        clr(q, WBIT[C][i]);
    }
}
inline void cross_off(u8* seg, u32 nbytes, SievingPrime& s) {
    switch (s.cls) {
        case 0: return cross_cls<0>(seg, nbytes, s);
        case 1: return cross_cls<1>(seg, nbytes, s);
        case 2: return cross_cls<2>(seg, nbytes, s);
        case 3: return cross_cls<3>(seg, nbytes, s);
        case 4: return cross_cls<4>(seg, nbytes, s);
        case 5: return cross_cls<5>(seg, nbytes, s);
        case 6: return cross_cls<6>(seg, nbytes, s);
        default: return cross_cls<7>(seg, nbytes, s);
    }
}

// 語 x の中で k 番目 (0 始まり) に立っているビットの位置
inline u32 kth_bit(u64 x, u32 k) {
#if defined(__x86_64__) && defined(__BMI2__)
    return (u32)std::countr_zero(_pdep_u64(u64(1) << k, x));
#else
    for (; k; --k) x &= x - 1;
    return (u32)std::countr_zero(x);
#endif
}

// バイト g の型が g mod (p q) で決まる、p と q の倍数を除く型。8 バイトずつ読めるように 1 周より 7 バイト長く持つ。
struct Pattern {
    std::vector<u8> b;
    u32 period;
    Pattern(u32 p, u32 q) : b(p * q + 7), period(p * q) {
        for (u32 g = 0; g < period + 7; ++g) {
            u8 v = 0;
            for (u32 i = 0; i < 8; ++i) {
                const u64 n = 30 * (u64)(g % period) + WR[i];
                v |= (u8)((n % p != 0 && n % q != 0) << i);
            }
            b[g] = v;
        }
    }
};

template <u32 SW = 4096>
inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) {
    std::vector<u32> sel;
    u64 cnt = 0, next = B;  // cnt は数えた素数の個数、next は次に出す素数の番号 (0 始まり)
    auto take = [&](u64 v) {
        if (cnt == next) sel.push_back((u32)v), next += A;
        ++cnt;
    };
    for (u32 p : {2u, 3u, 5u})
        if (p <= N) take(p);
    if (N < 7) return {(u32)cnt, sel};
    // √N までの素数 (奇数だけの篩)
    u32 sq = (u32)std::sqrt((double)N);
    while ((u64)sq * sq > N) --sq;
    while ((u64)(sq + 1) * (sq + 1) <= N) ++sq;
    std::vector<u32> sp;
    {
        std::vector<u8> comp(sq / 2 + 1, 0);
        for (u32 i = 1; (2 * i + 1) * (2 * i + 1) <= sq; ++i)
            if (!comp[i])
                for (u32 j = (2 * i + 1) * (2 * i + 1) / 2; j <= sq / 2; j += 2 * i + 1) comp[j] = 1;
        for (u32 i = 3; i <= sq / 2; ++i)  // 7 以上
            if (!comp[i] && 2 * i + 1 > 37) sp.push_back(2 * i + 1);
    }
    std::vector<SievingPrime> st(sp.size());
    for (size_t k = 0; k < sp.size(); ++k) {
        const u32 p = sp[k];
        st[k] = SievingPrime{(u32)((u64)p * p / 30), p / 30, BI[p % 30], BI[p % 30]};
    }
    // 7、11、13 の型と、2 つずつまとめた型
    std::vector<u8> pat(1001);
    for (u32 k = 0; k < 1001; ++k)
        for (u32 i = 0; i < 8; ++i) {
            const u64 n = 30 * (u64)k + WR[i];
            if (n % 7 && n % 11 && n % 13) pat[k] |= (u8)(1u << i);
        }
    const Pattern pq[3] = {Pattern(17, 19), Pattern(23, 29), Pattern(31, 37)};
    constexpr u64 SEGN = 240 * (u64)SW;
    std::vector<u64> seg64(SW + 1, 0);
    u8* seg = reinterpret_cast<u8*>(seg64.data());
    for (u64 low = 0; low <= N; low += SEGN) {
        const u64 high = std::min<u64>(low + SEGN, (u64)N + 1);
        const u32 nbytes = (u32)((high - low + 29) / 30), nw = (nbytes + 7) / 8;
        for (u32 k = 0, r = (u32)(low / 30 % 1001); k < nbytes; r = 0) {
            const u32 len = std::min<u32>(1001 - r, nbytes - k);
            std::memcpy(seg + k, pat.data() + r, len), k += len;
        }
        std::memset(seg + nbytes, 0, 8 * (SW + 1) - nbytes);
        for (const Pattern& P : pq)
            for (u32 k = 0, o = (u32)(low / 30 % P.period); k < nw; ++k) {
                u64 m;
                std::memcpy(&m, P.b.data() + o, 8);
                seg64[k] &= m;
                o += 8, o -= o >= P.period ? P.period : 0;
            }
        for (SievingPrime& s : st) cross_off(seg, nbytes, s);
        if (low == 0) {
            seg[0] &= (u8)~1u;  // 1 は素数でない
            for (u32 p : {7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u})
                if (p <= N) seg[p / 30] |= (u8)(1u << BI[p % 30]);  // 型で消した素数を戻す
        }
        // high 以上の数のビットを落とす
        const u64 last = high - 1 - low;
        u64 mlast = 0;
        for (u32 k = 0; k < 8; ++k)
            for (u32 i = 0; i < 8; ++i)
                if (30 * k + WR[i] <= last % 240) mlast |= u64(1) << (8 * k + i);
        seg64[last / 240] &= mlast;
        for (u32 w = last / 240 + 1; w < nw; ++w) seg64[w] = 0;
        // 数えて、A 個おきに取り出す
        for (u32 w = 0; w < nw; ++w) {
            const u64 x = seg64[w];
            const u32 pc = (u32)std::popcount(x);
            while (next < cnt + pc) {
                const u32 bit = kth_bit(x, (u32)(next - cnt));
                sel.push_back((u32)(low + 240 * (u64)w + 30 * (bit >> 3) + WR[bit & 7]));
                next += A;
            }
            cnt += pc;
        }
    }
    return {(u32)cnt, sel};
}

}  // namespace ep1
