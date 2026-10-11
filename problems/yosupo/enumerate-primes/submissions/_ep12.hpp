#pragma once
// 素数の列挙の 12 回目。11 回目の取り出しの 1 段目で、次に取り出す番号を語ごとに比べて進める代わりに、語の終わりまでの
// popcount の累積 c から「番号が c 未満の取り出しの数」floor((c + A - 1 - t0) / A) を直に出す (t0 < A は区画の中の最初の
// 番号)。割り算は M = ceil(2^64 / A) を掛けた上位 64 bit で行う (32 bit の被除数で正確)。語ごとの値は前の語の比べに
// よらないので、語をまたぐ依存は累積の足し算だけになる。1 語に 2 個以上を取り出すときだけ分岐で列を埋める。
// 以下は 11 回目の説明。10 回目の取り出しの機械語を直す。8 回目から 10 回目は、GCC が next と cnt をメモリに置いたまま
// 語ごとに読み書きしていて (take が参照で捕まえるため)、語ごとにメモリを経た依存の鎖ができていた。9 回目と 10 回目の
// 2 段の取り出しでは、GCC が「取り出す素数があるか」の選択を分岐に戻していた。11 回目は区画の中の数を手元の変数で回し、
// 次に取り出す番号の更新をマスクの算術で書く。A ≥ 16 なら区画の中で取り出す数は 4 nw + 1 個以下なので、1 段目の列は
// 溢れない (溢れの確かめを輪から除く)。A < 16 は語ごとに素直に取り出す。
// 以下は 10 回目の説明。9 回目の形で、小区画で消す小さい素数 (PSPLIT 以下) の倍数を 210 の車輪で進めて、7 の倍数の m を
// 飛ばす (消す数が 7 分の 1 減る)。m が 210 進む 7 周 (56 個から 7 の倍数の 8 個を除いた 48 個) をまとめて消す。周の先頭を
// m ≡ 1 (mod 210) にそろえると、飛ばす位置は p によらず、周 r の中で (2r + WR[k]) mod 7 = 0 の k になる。7 周の中は、
// 周の先頭のポインタを p ずつ進め、周の中の 8 個の差 (30 の車輪と同じ) を使い回す。最初のまとまりは
// m = 210 floor(p / 210) + 1 から始める。p < 210 なら m = 1 で p 自身を消すので、最初の区画で戻す。はみ出しは 7p バイト
// 未満になるので、7 PSPLIT が小区画と余白に収まるようにする。
// 以下は 9 回目の説明。8 回目の形で、A 個おきの素数の取り出しを 2 段に分ける。1 段目は語を前から見て、popcount の累積と
// 次に取り出す番号を比べ、取り出す素数がある語の番号と、その語より前の個数を列に書く (分岐なし。取り出す素数が無い語
// でも書いて、列の長さを進めない)。2 段目は列の各項で語の中の k 番目のビットを引く。2 段目は項どうしが独立なので、
// 8 回目のように語ごとに k 番目のビットを引くより命令が少なく、並べて実行できる。A < A2 (1 語に 2 個以上を取り出すことが
// 多い) なら 8 回目の形で取り出す。
// 以下は 8 回目の説明。7 回目の形で、倍数を消す関数の出入りの手間を減らす。7 回目までは素数ごとに「今の区画の先頭が周の
// 何番目か」を持ち、周の途中から始めて周の途中で終わるので、出入りのたびに輪の出口の分岐が 2〜3 回外れていた。8 回目は
// 周の先頭だけを持ち、区画の中に先頭がある周を 8 個ずつ丸ごと消す。周の最後は区画の後ろへ p バイト未満はみ出す。
// 小区画どうしのはみ出しは、次の小区画の型を先に書いておいて受ける。区画の後ろへのはみ出しは、余白を全部 1 にして
// 受け、次の区画の先頭に AND する。周の先頭は p^2 を含む周の先頭 (p / 30) (p + 1) バイトから始める (その周の
// p^2 より前の倍数も合成数)。
// 以下は 7 回目の説明。6 回目の形で、A 個おきの素数の取り出し方を変える。6 回目までは語ごとに「この語に取り出す素数が
// あるか」で分岐していて、A = 27 なら 2 語に 1 回ほどの割合で外れる。A < ASEL なら、語ごとに 1 個を分岐なしで取り出し
// (取り出す素数が無い語でも書いて、数を進めない)、同じ語に 2 個目からがあるときだけ輪で取り出す。語の中の k 番目の
// ビットは、BMI2 があれば pdep で、無ければバイトごとの累積の popcount と 256 通りの表で引く (分岐なし)。型は 8 本ずつ
// まとめて AND する。
// 以下は 6 回目の説明。5 回目 (_ep5.hpp) の二段の区画のまま、前もって消す型の掛け方と、倍数を消す素数の並べ方を変える。
// 型は 7 から PMAX までの素数を 2 つか 3 つずつ (周期が 64 KB 以下になるように) まとめる。小区画を 2 KB ずつに切り、
// 型 4 本 (7 回目は 8 本) の AND を 32 バイトずつまとめて書く (GCC のベクトル拡張で、x64 は AVX2、arm は NEON になる)。5 回目までは型
// 1 本ごとに区画を読み書きしていて、GCC はその輪をベクトル化しなかった。型は小区画ごとに書き、続けて小さい素数で
// 消すので、小区画が L1 にあるうちに済む。倍数を消す素数は p mod 30 の類で並べ、類ごとの関数を直に呼ぶ。
#include "../common.hpp"
#if defined(__x86_64__) && defined(__BMI2__)
#include <immintrin.h>
#endif

namespace ep12 {

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

// 篩う素数の状態。pos は次に消す周の先頭の、今の区画の先頭からのバイトの位置、a = p / 30。
struct SievingPrime {
    u32 pos, a;
};

// 区画の [0, nbytes) に先頭がある p の倍数の周を、8 個ずつ丸ごと消す (p ≡ WR[C] mod 30)。最後の周は nbytes から
// p バイト未満はみ出す。
template <u32 C>
inline void cross_cls(u8* seg, u32 nbytes, SievingPrime& s) {
    const u32 a = s.a, p = 30 * a + WR[C];
    const u32 o1 = a * 6 + WOFF[C][1], o2 = a * 10 + WOFF[C][2], o3 = a * 12 + WOFF[C][3], o4 = a * 16 + WOFF[C][4];
    const u32 o5 = a * 18 + WOFF[C][5], o6 = a * 22 + WOFF[C][6], o7 = a * 28 + WOFF[C][7];
    constexpr u8 m0 = (u8)~(1u << WBIT[C][0]), m1 = (u8)~(1u << WBIT[C][1]), m2 = (u8)~(1u << WBIT[C][2]), m3 = (u8)~(1u << WBIT[C][3]);
    constexpr u8 m4 = (u8)~(1u << WBIT[C][4]), m5 = (u8)~(1u << WBIT[C][5]), m6 = (u8)~(1u << WBIT[C][6]), m7 = (u8)~(1u << WBIT[C][7]);
    u32 base = s.pos;
    for (; base < nbytes; base += p) {
        u8* q = seg + base;
        q[0] &= m0, q[o1] &= m1, q[o2] &= m2, q[o3] &= m3, q[o4] &= m4, q[o5] &= m5, q[o6] &= m6, q[o7] &= m7;
    }
    s.pos = base - nbytes;
}
// st[beg[c], beg[c + 1]) が類 c の素数。類 C から 7 までを順に消す。
template <u32 C = 0>
inline void cross_all(u8* seg, u32 nbytes, SievingPrime* st, const u32* beg) {
    for (u32 k = beg[C]; k < beg[C + 1]; ++k) cross_cls<C>(seg, nbytes, st[k]);
    if constexpr (C < 7) cross_all<C + 1>(seg, nbytes, st, beg);
}

// 7 周のまとまりの周 R で、周の先頭 q から 8 個の差 o の位置を消す。m が 7 の倍数になる k ((2R + WR[k]) mod 7 = 0) は飛ばす。
template <u32 C, u32 R, u32... K>
inline void row210(u8* q, const u32* o, std::integer_sequence<u32, K...>) {
    ((((2 * R + WR[K]) % 7 != 0) ? (void)(q[o[K]] &= (u8)~(1u << WBIT[C][K])) : (void)0), ...);
}
// 区画の [0, nbytes) に先頭がある p の倍数の 7 周のまとまりを丸ごと消す (p ≡ WR[C] mod 30)。pos はまとまりの先頭
// (m ≡ 1 mod 210) の位置。最後のまとまりは nbytes から 7p バイト未満はみ出す。
template <u32 C>
inline void cross210_cls(u8* seg, u32 nbytes, SievingPrime& s) {
    const u32 a = s.a, p = 30 * a + WR[C];
    const u32 o[8] = {0, a * 6 + WOFF[C][1], a * 10 + WOFF[C][2], a * 12 + WOFF[C][3],
                      a * 16 + WOFF[C][4], a * 18 + WOFF[C][5], a * 22 + WOFF[C][6], a * 28 + WOFF[C][7]};
    constexpr auto K8 = std::make_integer_sequence<u32, 8>{};
    u32 base = s.pos;
    for (; base < nbytes; base += 7 * p) {
        u8* q = seg + base;
        row210<C, 0>(q, o, K8), q += p;
        row210<C, 1>(q, o, K8), q += p;
        row210<C, 2>(q, o, K8), q += p;
        row210<C, 3>(q, o, K8), q += p;
        row210<C, 4>(q, o, K8), q += p;
        row210<C, 5>(q, o, K8), q += p;
        row210<C, 6>(q, o, K8);
    }
    s.pos = base - nbytes;
}
template <u32 C = 0>
inline void cross210_all(u8* seg, u32 nbytes, SievingPrime* st, const u32* beg) {
    for (u32 k = beg[C]; k < beg[C + 1]; ++k) cross210_cls<C>(seg, nbytes, st[k]);
    if constexpr (C < 7) cross210_all<C + 1>(seg, nbytes, st, beg);
}

// 語のビット b (0 から 63) が表す数の、語の先頭からの差。64 は立っているビットが足りないときの捨てる値。
inline constexpr std::array<u8, 65> BITVAL = [] {
    std::array<u8, 65> t{};
    for (u32 b = 0; b < 64; ++b) t[b] = (u8)(30 * (b >> 3) + WR[b & 7]);
    return t;
}();
#if !(defined(__x86_64__) && defined(__BMI2__))
// バイト b の中で r 番目に立っているビットの位置
inline constexpr std::array<std::array<u8, 8>, 256> SEL8 = [] {
    std::array<std::array<u8, 8>, 256> t{};
    for (u32 b = 0; b < 256; ++b)
        for (u32 i = 0, r = 0; i < 8; ++i)
            if (b >> i & 1) t[b][r++] = (u8)i;
    return t;
}();
#endif
// 語 x の中で k 番目 (0 始まり、k < 64) に立っているビットの位置。立っているビットが k 個以下なら、0 から 64 の何か。
inline u32 kth_bit(u64 x, u32 k) {
#if defined(__x86_64__) && defined(__BMI2__)
    return (u32)std::countr_zero(_pdep_u64(u64(1) << k, x));
#else
    constexpr u64 L8 = 0x0101010101010101ull, H8 = 0x8080808080808080ull;
    u64 s = x - ((x >> 1) & 0x5555555555555555ull);
    s = (s & 0x3333333333333333ull) + ((s >> 2) & 0x3333333333333333ull);
    s = (s + (s >> 4)) & 0x0f0f0f0f0f0f0f0full;
    const u64 pre = s * L8;                         // バイト i は、バイト 0 から i までの popcount
    const u64 le = ((k * L8 | H8) - pre) & H8;      // 累積が k 以下のバイトに最上位ビットを立てる
    const u32 sh = (u32)(((le >> 7) * L8 >> 56) * 8) & 63;  // k 番目のビットがあるバイトの位置
    const u32 before = (u32)((pre << 8) >> sh) & 0xff;      // それより前のバイトの popcount
    return sh + SEL8[(x >> sh) & 0xff][(k - before) & 7];
#endif
}

// 7 から pmax までの素数の倍数を除く型。バイト g の型は g mod (まとめた素数の積) で決まる。1 周の先に CH + 32 バイト
// 足して持ち、途中から CH バイトを続けて読めるようにする。
struct Presieve {
    static constexpr u32 CH = 2048;  // 1 回に書くバイト数
    static constexpr u32 G = 8;      // 1 回の読み書きで AND する型の本数
    using V = u8 __attribute__((vector_size(32)));
    struct Pat {
        std::vector<u8> b;
        u32 period, step;  // step = CH mod period
    };
    std::vector<Pat> pats;  // G の倍数本 (足りない分は全部 1 の型)
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
        while (pats.size() % G) add({});
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
            for (size_t k = 0; k < np; k += G) {
                const u8* p[G];
                for (u32 t = 0; t < G; ++t) p[t] = pats[k + t].b.data() + off[k + t];
                if (k == 0)
                    for (u32 j = 0; j < nv; ++j) {
                        V v = ld(p[0] + 32 * j);
#pragma GCC unroll 16
                        for (u32 t = 1; t < G; ++t) v &= ld(p[t] + 32 * j);
                        std::memcpy(out + 32 * j, &v, 32);
                    }
                else
                    for (u32 j = 0; j < nv; ++j) {
                        V v = ld(out + 32 * j);
#pragma GCC unroll 16
                        for (u32 t = 0; t < G; ++t) v &= ld(p[t] + 32 * j);
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
// 2 段目: wc[0, n) の各項 (語の番号と、その語より前の個数) で、語の中の k 番目のビットを引いて sel に足す。項 i は区画の
// 中の番号 t0 + i A。
inline void select_words(const u64* wc, u32 n, u32 t0, u32 A, u32 low, const u64* seg64, std::vector<u32>& sel) {
    const size_t z = sel.size();
    sel.resize(z + n);
    u32* out = sel.data() + z;
    for (u32 i = 0; i < n; ++i) {
        const u32 w = (u32)wc[i], k = t0 + i * A - (u32)(wc[i] >> 32);
        out[i] = low + 240 * w + BITVAL[kth_bit(seg64[w], k)];
    }
}

template <u32 SW = 8192, u32 SWS = 2048, u32 PSPLIT = 1000, u32 PMAX = 163>
inline std::pair<u32, std::vector<u32>> run(u32 N, u32 A, u32 B) {
    static_assert(SWS % 4 == 0 && SW % SWS == 0 && 7 * PSPLIT <= 8 * SWS);
    std::vector<u32> sel;
    // π(N) < 1.26 N / ln N (N > 1) から取り出す数の上を見積もって、先に確保する (触らないページは載らない)
    sel.reserve(N < 17 ? 8 : (size_t)(1.26 * N / std::log((double)N)) / A + 8);
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
    // 小さい素数と大きい素数を、それぞれ類ごとに並べる。p^2 を含む周から消す。
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
            // 小さい素数は m = 210 floor(p / 210) + 1 のまとまりから、大きい素数は p^2 を含む周から
            if (k < nsmall) st[ps[c]++] = SievingPrime{7 * p * (p / 210) + p / 30, p / 30};
            else st[pl[c]++] = SievingPrime{p / 30 * (p + 1), p / 30};
        }
    }
    const Presieve pre(PMAX);
    constexpr u64 SEGN = 240 * (u64)SW;
    // 区画の後ろの余白 (周のはみ出しは p バイト未満) と、次の区画の先頭に AND するはみ出し
    const u32 SL = (std::max(sq, 7 * PSPLIT) + 64) / 64 * 64;
    std::vector<u64> seg64(SW + SL / 8 + 8, 0), carry(SL / 8, ~u64(0));
    const std::unique_ptr<u64[]> wc(new u64[4 * SW + 64]);  // 取り出す素数がある語の番号と、その語より前の個数
    const u64 MA = A >= 2 ? ~u64(0) / A + 1 : 0;              // ceil(2^64 / A)。A < 16 では使わない
    u8* seg = reinterpret_cast<u8*>(seg64.data());
    for (u64 low = 0; low <= N; low += SEGN) {
        const u64 high = std::min<u64>(low + SEGN, (u64)N + 1);
        const u32 nbytes = (u32)((high - low + 29) / 30), nw = (nbytes + 7) / 8;
        // 小区画は、次の小区画の型を先に書いてから消す。最後の小区画の後ろは余白を全部 1 にする
        constexpr u32 SB = 8 * SWS;
        pre.fill(seg, std::min(SB, nbytes), low / 30);
        for (u32 o = 0; o < nbytes; o += SB) {
            if (o + SB < nbytes) pre.fill(seg + o + SB, std::min(SB, nbytes - o - SB), low / 30 + o + SB);
            else std::memset(seg + nbytes, 0xff, SL);
            cross210_all(seg + o, std::min(SB, nbytes - o), st.data(), bs);
        }
        for (u32 i = 0; i < SL / 8; ++i) seg64[i] &= carry[i];  // 前の区画からのはみ出し
        cross_all(seg, nbytes, st.data(), bl);
        std::memcpy(carry.data(), seg + nbytes, SL);
        if (low == 0) {
            seg[0] &= (u8)~1u;  // 1 は素数でない
            for (u32 p = 7; p <= std::max(PMAX, 210u) && p <= N; p += 2) {  // 型と 210 の車輪の m = 1 で消した素数を戻す
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
        // 数えて、A 個おきに取り出す (next - cnt < A は B < A なら成り立つ)
        if (A >= 16 && next - cnt < A) {
            // 1 段目: 取り出す素数がある語の番号と、その語より前の個数を wc に書く (取り出す素数が無い語でも書いて、n を
            // 進めない)。tn は次に取り出す、区画の中の番号
            const u32 t0 = (u32)(next - cnt);  // < A
            const u32 K = A - 1 - t0;
            u32 c = 0, n = 0;
            u64* const W = wc.get();
            for (u32 w = 0; w < nw; ++w) {
                const u32 c1 = c + (u32)std::popcount(seg64[w]);
                const u64 e = (u64)c << 32 | w;
                W[n] = e;
                const u32 n1 = (u32)((unsigned __int128)(c1 + K) * MA >> 64);  // 番号が c1 未満の取り出しの数
                if (n1 > n + 1) [[unlikely]]
                    for (u32 j = n + 1; j < n1; ++j) W[j] = e;  // 同じ語の 2 個目から
                n = n1, c = c1;
            }
            select_words(W, n, t0, A, (u32)low, seg64.data(), sel);
            next = cnt + t0 + (u64)n * A, cnt += c;
        } else {
            u64 cn = cnt, nx = next;
            for (u32 w = 0; w < nw; ++w) {
                const u64 x = seg64[w];
                const u32 pc = (u32)std::popcount(x);
                for (; nx < cn + pc; nx += A) sel.push_back((u32)low + 240 * w + BITVAL[kth_bit(x, (u32)(nx - cn))]);
                cn += pc;
            }
            cnt = cn, next = nx;
        }
    }
    return {(u32)cnt, sel};
}

}  // namespace ep12
