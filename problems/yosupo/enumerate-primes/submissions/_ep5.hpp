#pragma once
// 素数の列挙の 5 回目。3 回目 (_ep3.hpp) の 30 の車輪の形で、倍数を消す素数を 2 つに分ける。PSPLIT 以下の素数は区画を
// SWS 語の小区画 (L1 に載る大きさ) に分けて小区画ごとに消し、それより大きい素数は SW 語の区画全体で消す。小さい素数は
// 倍数が多く、区画が L1 に載らないと読み書きが L2 に出る。大きい素数は小区画ごとに出入りすると手間が重い。
// 以下は 3 回目と 2 回目の説明。型は 1 回目 (_ep1.hpp) の型で消す素数を PMAX まで増やし、1 本のバイト列から非整列で読む。型は素数を 2 つか 3 つずつまとめ (周期は
// 積のバイト数)、区画の中で周期が途切れない範囲ごとに 8 バイトずつの AND の輪で回す (コンパイラがベクトル化できる)。
// 以下は 1 回目の説明。素数の個数 (NeoLibrary 7b624cb の prime_pi) の篩を列挙の形にしたもの。30 の車輪のビット列を
// 区画 (SW 語) ごとに作る。7、11、13 は 1001 バイト周期の型を写し、17 と 19、23 と 29、31 と 37 は 2 つずつまとめた
// 周期の型を AND し、41 以上 √N 以下の素数は倍数の車輪の 1 周 8 個を並べて p^2 から消す。区画ごとに popcount で数え、
// B 番目から A 個おきの素数は、語ごとの個数を見て、語の中の k 番目のビットを pdep で引いて取り出す。
#include "../common.hpp"
#if defined(__x86_64__) && defined(__BMI2__)
#include <immintrin.h>
#endif

namespace ep5 {

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

// バイト g の型が g mod (素数の積) で決まる、ps の倍数を除く型。1 周の先に 8 バイト足して持ち、途中から非整列で読む。
struct Pattern {
    std::vector<u8> b;
    u32 period;  // バイト
    explicit Pattern(std::initializer_list<u32> ps) {
        period = 1;
        for (u32 p : ps) period *= p;
        b.resize(period + 8);
        for (u32 g = 0; g < period + 8; ++g) {
            u8 v = 0;
            for (u32 i = 0; i < 8; ++i) {
                const u64 n = 30 * (u64)(g % period) + WR[i];
                bool ok = true;
                for (u32 p : ps) ok &= n % p != 0;
                v |= (u8)(ok << i);
            }
            b[g] = v;
        }
    }
    // seg64[0, nw) に、区画の先頭のバイトが g0 mod period の型を AND する
    void apply(u64* seg64, u32 nw, u64 g0) const {
        u32 o = (u32)(g0 % period);
        for (u32 k = 0; k < nw;) {
            const u32 run = std::min<u32>(nw - k, std::max<u32>(1, (period - o) / 8));
            const u8* src = b.data() + o;
            for (u32 j = 0; j < run; ++j) {
                u64 m;
                std::memcpy(&m, src + 8 * j, 8);
                seg64[k + j] &= m;
            }
            k += run, o += 8 * run;
            if (o >= period) o -= period;
        }
    }
};

template <u32 SW = 16384, u32 SWS = 4096, u32 PSPLIT = 2000, u32 PMAX = 101>
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
            if (!comp[i] && 2 * i + 1 > PMAX) sp.push_back(2 * i + 1);
    }
    std::vector<SievingPrime> st;
    for (const u32 p : sp) st.push_back(SievingPrime{(u32)((u64)p * p / 30), p / 30, BI[p % 30], BI[p % 30]});  // p^2 から消す
    const size_t nsmall = (size_t)(std::upper_bound(sp.begin(), sp.end(), PSPLIT) - sp.begin());
    // 7、11、13 の型と、2 つずつまとめた型
    std::vector<u8> pat(1001);
    for (u32 k = 0; k < 1001; ++k)
        for (u32 i = 0; i < 8; ++i) {
            const u64 n = 30 * (u64)k + WR[i];
            if (n % 7 && n % 11 && n % 13) pat[k] |= (u8)(1u << i);
        }
    // 17 から PMAX までの素数を 3 つずつ (周期が 64 KB を超えるなら 2 つ) まとめた型
    std::vector<Pattern> pq;
    {
        std::vector<u32> small;
        for (u32 p = 17; p <= PMAX; p += 2) {
            bool pr = true;
            for (u32 d = 3; d * d <= p; d += 2) pr &= p % d != 0;
            if (pr) small.push_back(p);
        }
        for (size_t i = 0; i < small.size();) {
            if (i + 2 < small.size() && (u64)small[i] * small[i + 1] * small[i + 2] <= 65536) pq.emplace_back(Pattern{small[i], small[i + 1], small[i + 2]}), i += 3;
            else if (i + 1 < small.size()) pq.emplace_back(Pattern{small[i], small[i + 1]}), i += 2;
            else pq.emplace_back(Pattern{small[i]}), i += 1;
        }
    }
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
        for (const Pattern& P : pq) P.apply(seg64.data(), nw, low / 30);
        // PSPLIT 以下の素数は小区画ごと、それより大きい素数は区画全体で消す
        for (u32 o = 0; o < nbytes; o += 8 * SWS) {
            const u32 len = std::min<u32>(8 * SWS, nbytes - o);
            for (size_t k = 0; k < nsmall; ++k) cross_off(seg + o, len, st[k]);
        }
        for (size_t k = nsmall; k < st.size(); ++k) cross_off(seg, nbytes, st[k]);
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

}  // namespace ep5
