#pragma once
// 素数の列挙の 6 回目。5 回目 (_ep5.hpp) の二段の区画のまま、前もって消す型の掛け方と、倍数を消す素数の並べ方を変える。
// 型は 7 から PMAX までの素数を 2 つか 3 つずつ (周期が 64 KB 以下になるように) まとめる。小区画を 2 KB ずつに切り、
// 型 4 本の AND を 32 バイトずつまとめて書く (GCC のベクトル拡張で、x64 は AVX2、arm は NEON になる)。5 回目までは型
// 1 本ごとに区画を読み書きしていて、GCC はその輪をベクトル化しなかった。型は小区画ごとに書き、続けて小さい素数で
// 消すので、小区画が L1 にあるうちに済む。倍数を消す素数は p mod 30 の類で並べ、類ごとの関数を直に呼ぶ。
#include "../common.hpp"
#if defined(__x86_64__) && defined(__BMI2__)
#include <immintrin.h>
#endif

namespace ep6 {

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

// 篩う素数の状態。pos は今の区画の先頭からのバイトの位置、a = p / 30、i は倍数 p m の m の車輪の位置。
struct SievingPrime {
    u32 pos, a, i;
};

// 区画の [0, nbytes) バイトにある p の倍数を消す (p ≡ WR[C] mod 30)。周の途中は 1 つずつ、周が丸ごと入るところは
// 8 個を並べて消す。
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
            s.pos = q - nbytes, s.i = i;
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
            s.pos = q - nbytes, s.i = i;
            return;
        }
        clr(q, WBIT[C][i]);
    }
}
// st[beg[c], beg[c + 1]) が類 c の素数。類 C から 7 までを順に消す。
template <u32 C = 0>
inline void cross_all(u8* seg, u32 nbytes, SievingPrime* st, const u32* beg) {
    for (u32 k = beg[C]; k < beg[C + 1]; ++k) cross_cls<C>(seg, nbytes, st[k]);
    if constexpr (C < 7) cross_all<C + 1>(seg, nbytes, st, beg);
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

// 7 から pmax までの素数の倍数を除く型。バイト g の型は g mod (まとめた素数の積) で決まる。1 周の先に CH + 32 バイト
// 足して持ち、途中から CH バイトを続けて読めるようにする。
struct Presieve {
    static constexpr u32 CH = 2048;  // 1 回に書くバイト数
    using V = u8 __attribute__((vector_size(32)));
    struct Pat {
        std::vector<u8> b;
        u32 period, step;  // step = CH mod period
    };
    std::vector<Pat> pats;  // 4 の倍数本 (足りない分は全部 1 の型)
    explicit Presieve(u32 pmax) {
        std::vector<u32> ps;
        for (u32 p = 7; p <= pmax; p += 2) {
            bool pr = true;
            for (u32 d = 3; d * d <= p; d += 2) pr &= p % d != 0;
            if (pr) ps.push_back(p);
        }
        for (size_t i = 0; i < ps.size();) {
            size_t j = i + 1;
            u64 prod = ps[i];
            while (j < ps.size() && j < i + 3 && prod * ps[j] <= 65536) prod *= ps[j++];
            add(std::vector<u32>(ps.begin() + (std::ptrdiff_t)i, ps.begin() + (std::ptrdiff_t)j));
            i = j;
        }
        while (pats.size() % 4) add({});
    }
    void add(const std::vector<u32>& g) {
        Pat P;
        P.period = 1;
        for (u32 p : g) P.period *= p;
        const u32 len = P.period + CH + 32;
        P.b.assign(len, 0xff);
        for (u32 p : g) {
            std::vector<u8> one(p, 0xff);  // p の倍数を除く 1 周 p バイトの型
            for (u32 i = 0; i < 8; ++i) {
                u32 h = 0;
                while ((30 * h + WR[i]) % p) ++h;
                one[h] &= (u8)~(1u << i);
            }
            for (u32 o = 0; o < len; o += p) {
                const u32 n = std::min(p, len - o);
                for (u32 k = 0; k < n; ++k) P.b[o + k] &= one[k];
            }
        }
        P.step = CH % P.period;
        pats.push_back(std::move(P));
    }
    // seg[0, len) に、先頭のバイトが g0 の型を書く。len を 32 の倍数に切り上げた所まで書く。
    void fill(u8* seg, u32 len, u64 g0) const {
        const size_t np = pats.size();
        assert(np <= 64);
        u32 off[64];
        for (size_t k = 0; k < np; ++k) off[k] = (u32)(g0 % pats[k].period);
        auto ld = [](const u8* p) {
            V v;
            std::memcpy(&v, p, 32);
            return v;
        };
        for (u32 c = 0; c < len; c += CH) {
            const u32 nv = (std::min(CH, len - c) + 31) / 32;
            u8* out = seg + c;
            for (size_t k = 0; k < np; k += 4) {
                const u8 *p0 = pats[k].b.data() + off[k], *p1 = pats[k + 1].b.data() + off[k + 1];
                const u8 *p2 = pats[k + 2].b.data() + off[k + 2], *p3 = pats[k + 3].b.data() + off[k + 3];
                if (k == 0)
                    for (u32 j = 0; j < nv; ++j) {
                        const V v = ld(p0 + 32 * j) & ld(p1 + 32 * j) & ld(p2 + 32 * j) & ld(p3 + 32 * j);
                        std::memcpy(out + 32 * j, &v, 32);
                    }
                else
                    for (u32 j = 0; j < nv; ++j) {
                        const V v = ld(out + 32 * j) & ld(p0 + 32 * j) & ld(p1 + 32 * j) & ld(p2 + 32 * j) & ld(p3 + 32 * j);
                        std::memcpy(out + 32 * j, &v, 32);
                    }
            }
            for (size_t k = 0; k < np; ++k) {
                off[k] += pats[k].step;
                if (off[k] >= pats[k].period) off[k] -= pats[k].period;
            }
        }
    }
};

// SW 語の区画を SWS 語の小区画に分け、小区画ごとに型を書いて PSPLIT 以下の素数で消し、区画全体を PSPLIT より大きい
// 素数で消す。PMAX 以下の素数は型で消す。
template <u32 SW = 32768, u32 SWS = 2048, u32 PSPLIT = 1000, u32 PMAX = 163>
inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) {
    static_assert(SWS % 4 == 0 && SW % SWS == 0);
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
        for (u32 i = 3; i <= sq / 2; ++i)
            if (!comp[i] && 2 * i + 1 > PMAX) sp.push_back(2 * i + 1);
    }
    // 小さい素数と大きい素数を、それぞれ類ごとに並べる。p^2 から消す。
    std::vector<SievingPrime> st(sp.size());
    u32 bs[9] = {}, bl[9] = {};
    {
        const u32 nsmall = (u32)(std::upper_bound(sp.begin(), sp.end(), PSPLIT) - sp.begin());
        u32 cs[8] = {}, cl[8] = {};
        for (u32 k = 0; k < sp.size(); ++k) ++(k < nsmall ? cs : cl)[BI[sp[k] % 30]];
        for (u32 c = 0; c < 8; ++c) bs[c + 1] = bs[c] + cs[c];
        bl[0] = nsmall;
        for (u32 c = 0; c < 8; ++c) bl[c + 1] = bl[c] + cl[c];
        u32 ps[8], pl[8];
        for (u32 c = 0; c < 8; ++c) ps[c] = bs[c], pl[c] = bl[c];
        for (u32 k = 0; k < sp.size(); ++k) {
            const u32 p = sp[k], c = BI[p % 30];
            st[k < nsmall ? ps[c]++ : pl[c]++] = SievingPrime{(u32)((u64)p * p / 30), p / 30, c};
        }
    }
    const Presieve pre(PMAX);
    constexpr u64 SEGN = 240 * (u64)SW;
    std::vector<u64> seg64(SW + 8, 0);
    u8* seg = reinterpret_cast<u8*>(seg64.data());
    for (u64 low = 0; low <= N; low += SEGN) {
        const u64 high = std::min<u64>(low + SEGN, (u64)N + 1);
        const u32 nbytes = (u32)((high - low + 29) / 30), nw = (nbytes + 7) / 8;
        for (u32 o = 0; o < nbytes; o += 8 * SWS) {
            const u32 len = std::min<u32>(8 * SWS, nbytes - o);
            pre.fill(seg + o, len, low / 30 + o);
            cross_all(seg + o, len, st.data(), bs);
        }
        cross_all(seg, nbytes, st.data(), bl);
        if (low == 0) {
            seg[0] &= (u8)~1u;  // 1 は素数でない
            for (u32 p = 7; p <= PMAX && p <= N; p += 2) {  // 型で消した素数を戻す
                bool pr = p % 3 && p % 5;
                for (u32 d = 7; d * d <= p; d += 2) pr &= p % d != 0;
                if (pr) seg[p / 30] |= (u8)(1u << BI[p % 30]);
            }
        }
        // high 以上の数のビットを落とす
        const u64 last = high - 1 - low;
        u64 mlast = 0;
        for (u32 k = 0; k < 8; ++k)
            for (u32 i = 0; i < 8; ++i)
                if (30 * k + WR[i] <= last % 240) mlast |= u64(1) << (8 * k + i);
        seg64[last / 240] &= mlast;
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

}  // namespace ep6
