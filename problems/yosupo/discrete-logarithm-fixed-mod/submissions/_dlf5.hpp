#pragma once
// 法が固定の離散対数の 5 回目。4 回目の表を作る側で、p、p - 1、(p - 1) / 2、表の場所を局所変数に写し、mod (p - 1) の
// 足し算なども局所の関数にする。メンバのままだと、log の表への u32 の書き込みと別名になりうるので、gcc が段ごとの輪の
// 中で毎回メモリから読み直していた (EPYC 7763 の x64-gcc で 4 回目の手直しが効かなかった)。
//
// 以下は 4 回目の説明。法が固定の離散対数の 4 回目。3 回目の p mod i の段で、M6 なら 2 か 3 で割り切れる i を log 2 + log(i / 2) か
// log 3 + log(i / 3) で埋める。i / 2 と i / 3 は順に進むので読むのが速く、p mod i の飛び飛びの読みは 6 と互いに素な i
// (3 分の 1) だけになる。i mod 6 は 6 の周期で回るので分岐は当たる。M30 なら 5 で割り切れる i も log 5 + log(i / 5) で
// 埋め (i mod 30 で種類を引く)、飛び飛びの読みは 30 と互いに素な i (15 分の 4) だけになる。M6U は M6 を 6 個ずつ
// 開いて種類の分岐をなくす (6 の倍数の i から 2、他、2、3、2、他の順)。
// 引く側は、表の場所と定数だけを持つ値の構造体 View で引く (呼ぶ側の局所変数にすると、答えの列への書き込みと別名に
// ならないので定数がレジスタに残る)。a ≤ LIM も Farey の表で引き (0/1 の区画に入る)、a = 0 は最後に選ぶ。
//
// 以下は 3 回目の説明。法が固定の離散対数の 3 回目。2 回目の 1 段目 (最初の K1 個の素数の Pohlig-Hellman) を次のように替える。
// - baby-step の表は z^j を 4 本の掛け算の鎖で作り、(鍵 << 32 | 添字) の 8 バイトの項を 4 個ずつの籠に置く (籠あたり
//   平均 1.2 個)。引くときは籠の 4 個を分岐なしで比べ、籠が埋まっているときだけ次の籠を見る。
// - giant-step は K1 個の的を並べて進め (見つかった的は外す)、掛け算の待ちと表を引く待ちを重ねる。
// - B は √(K1 q / 2) にする (2 回目は √(K1 q))。的ごとの giant-step の数は平均 q / (2B) なので、表を作る手間と釣り合う。
// - W5 なら p mod i の段で、2 段目で埋めた smooth な数も確かめずに書き直す (log は 1 つに決まるので値は同じ)。埋まって
//   いるかの分岐が外れなくなる。
//
// 以下は 2 回目の説明。法が固定の離散対数の 2 回目。1 回目から次を替える。
// - 2 段目の歩みを g^{-1} でなく c^{-1} (c = g^K、K は大きい奇数の定数) の掛け算にする。g = 2 のとき g^{-1} を掛けると
//   分数の分母が倍になるだけのことが多く、分子と分母の log が分かるまでの回数が 4 倍ほどになっていた。
// - smooth な数を広げる素数の上限 SM と、Pohlig-Hellman で求める素数の数 K1 を引数にする。
// - PF なら p mod i の段で、同じ p / i の続くあいだ PD 段先の log[p mod i] を先読みする。
// - SEG なら √p より大きい i を 2^15 個ずつの区画に分け、合成数は √LIM 以下の素数 a の倍数として log a + log(i / a) で
//   埋め (区画の大きさを区画の左端以下にすると i / a は前の区画に入る)、残った素数だけを p mod i の段で埋める。
//   p mod i の段は log[p mod i] を飛び飛びに読むが、倍数の段は i / a を順に読む。
//
// 以下は 1 回目の説明。法が固定の離散対数の 1 回目。index_calculus.hpp (yosupo の purplesyringa の提出の写し、maspy の 1 問 O(1) の方法) と
// 同じ手順で、次を替える。
// - a を Farey の表の区画に割り当てる floor(a NB / p) を、floor(2^64 NB / p) との掛け算の上位 64 bit で求める。誤差は
//   a 2^{-64} 未満で、a NB / p の小数部は 0 < a < p なら 1/p 以上なので、割り算と一致する。
// - p mod i の段 (i > √p で log i = log(p mod i) + log(-1) - log(p / i)) を、p / i が i ごとに高々 1 しか減らないことを
//   使って割り算なしで進める。
// - g^{-1} の掛け算と Pohlig-Hellman の累乗を Montgomery にする。mod (p - 1) の足し算は符号で補正する。
// - log の表と Farey の表を 2 MB 境界の mmap に置いて huge page を頼む (Linux)。答えの列も reserve してから頼み、0 で
//   埋めてから添字で書く。
//
// 以下は元の手順。a ≡ num / den (mod p) で |num| ≤ LIM、den < 2^16 となる分数を、a / p を NB 個に区切った区画ごとに
// Farey の表 (Stern-Brocot の中間数で埋める) で引き、log a = log |num| + [num < 0] (p-1)/2 - log den とする。log の表は
// LIM までの整数について前もって作る。最初の K1 個の素数の log は p - 1 の素因数ごとの Pohlig-Hellman (位数 r の
// baby-step giant-step を K1 個まとめて引く) で求め、√p までの残りの素数は、pp g^{-t} を Farey の表で分数にして分子と
// 分母の log が分かるまで t を進める。SM 以下の素数の冪を掛けた smooth な数の log も広げておく。√p までの合成数は最小の
// 素因数で、√p より大きい整数は p mod i の段で埋める。p < M0 なら g の冪を順にたどって全部の log を表にする。
#include "../common.hpp"
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace dlf5 {
using i32 = int32_t;

// 0 で埋めた bytes の領域。2 MB 以上なら Linux では 2 MB 境界の mmap に置いて huge page を頼む。
class Zeroed {
    void* p = nullptr;
#ifdef __linux__
    size_t mapped = 0;
#endif
   public:
    explicit Zeroed(size_t bytes) {
#ifdef __linux__
        constexpr size_t H = size_t(1) << 21;
        if (bytes >= H) {
            const size_t b = (bytes + 4095) & ~size_t(4095);
            void* m = mmap(nullptr, b + H, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (m != MAP_FAILED) {
                char *q = static_cast<char*>(m), *a = reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(q) + H - 1) & ~(H - 1));
                if (a != q) munmap(q, a - q);
                munmap(a + b, q + H - a);
                madvise(a, b, MADV_HUGEPAGE);
                p = a, mapped = b;
                return;
            }
        }
#endif
        p = calloc(bytes ? bytes : 1, 1);
    }
    ~Zeroed() {
#ifdef __linux__
        if (mapped) {
            munmap(p, mapped);
            return;
        }
#endif
        free(p);
    }
    Zeroed(const Zeroed&) = delete;
    Zeroed& operator=(const Zeroed&) = delete;
    void* get() const { return p; }
};
template <class T>
inline void reserve_huge(std::vector<T>& v, size_t n) {
    v.reserve(n);
#ifdef __linux__
    const uintptr_t b = reinterpret_cast<uintptr_t>(v.data()) & ~uintptr_t(4095), e = (reinterpret_cast<uintptr_t>(v.data() + n) + 4095) & ~uintptr_t(4095);
    if (b < e) madvise(reinterpret_cast<void*>(b), e - b, MADV_HUGEPAGE);
#endif
}

// R = 2^32 の Montgomery。値は [0, 2p) のまま持つ (p < 2^30)
struct Mont {
    u32 p, pinv, r1, r2;
    explicit Mont(u32 p_) : p(p_) {
        u32 x = (3 * p) ^ 2;
        for (int i = 0; i < 3; ++i) x *= 2 - p * x;
        pinv = x, r1 = (0u - p) % p, r2 = (u32)((u64)r1 * r1 % p);
    }
    u32 mul(u32 a, u32 b) const {
        const u64 t = (u64)a * b;
        return (u32)(t >> 32) + p - (u32)((u64)((u32)t * pinv) * p >> 32);
    }
    u32 to(u32 a) const { return mul(a, r2); }
    u32 norm(u32 a) const { return a >= p ? a - p : a; }
    u32 from(u32 a) const { return norm(mul(a, 1)); }
    u32 pow(u32 b, u64 e) const {  // b は Montgomery 形
        u32 r = r1;
        for (; e; e >>= 1, b = mul(b, b))
            if (e & 1) r = mul(r, b);
        return r;
    }
    u32 powi(u32 b, u64 e) const { return from(pow(to(b), e)); }  // 普通の形で
};

inline u32 inv_mod(u32 a, u32 m) {
    i64 x = 1, y = 0;
    u32 b = m;
    while (b) {
        const u32 t = a / b;
        a -= t * b, std::swap(a, b);
        x -= (i64)t * y, std::swap(x, y);
    }
    return (u32)(x < 0 ? x + m : x);
}

constexpr u32 M0 = 2000000, NB = 1000000, LIM = 1300000, SQ = 31624, WK = 0x2545F491u;  // WK は歩みの指数

// 位数 n の元 z の離散対数を引く baby-step giant-step。表は z^j (j < B) を (鍵 << 32 | j) で 4 個ずつの籠に前から詰めて
// 持ち、鍵は Montgomery 形を [0, p) に直した値 (0 にならないので 0 を空きに使う)
struct Bsgs {
    const Mont& M;
    u32 n, B, bmask, step;  // step = z^{-B} (Montgomery 形)
    std::vector<u64> tab;
    static u32 hash(u32 k) { return (k * 0x9E3779B1u) >> 8; }
    Bsgs(const Mont& M_, u32 z, u32 n_, u32 B_) : M(M_), n(n_), B(std::min(B_, n_)) {
        u32 nb = 4;
        while (nb * 6 < B * 5) nb *= 2;  // 籠あたり 1.2 個以下
        bmask = nb - 1;
        tab.assign(4 * (size_t)nb, 0);
        const u32 z1 = M.to(z), z2 = M.mul(z1, z1), z3 = M.mul(z2, z1), z4 = M.mul(z3, z1);
        u32 x[4] = {M.r1, z1, z2, z3};
        for (u32 j = 0; j < B; j += 4)
            for (u32 d = 0; d < 4; ++d) {
                if (j + d < B) {
                    const u32 k = M.norm(x[d]);
                    for (u32 b = hash(k) & bmask;; b = (b + 1) & bmask) {
                        u64* e = &tab[4 * (size_t)b];
                        const int t = (e[0] != 0) + (e[1] != 0) + (e[2] != 0) + (e[3] != 0);  // 前から詰めるので空きの先頭
                        if (t < 4) {
                            e[t] = (u64)k << 32 | (j + d);
                            break;
                        }
                    }
                }
                x[d] = M.mul(x[d], z4);
            }
        step = M.pow(z1, n - B);  // z^{-B}
    }
    u32 look(u32 k) const {
        for (u32 b = hash(k) & bmask;; b = (b + 1) & bmask) {
            const u64* e = &tab[4 * (size_t)b];
            u32 hit = ~0u;
            for (int t = 0; t < 4; ++t) hit = (u32)(e[t] >> 32) == k ? (u32)e[t] : hit;
            if (hit != ~0u || !e[3]) return hit;
        }
    }
    // z^{out[i]} = c[i] (c は普通の形) を cnt 個並べて求める
    void solve(const u32* c, u32 cnt, u32* out) const {
        u32 x[64], r[64], act[64], na = cnt;
        for (u32 i = 0; i < cnt; ++i) x[i] = M.to(c[i]), r[i] = 0, act[i] = i;
        while (na)
            for (u32 k = 0; k < na;) {
                const u32 i = act[k], v = look(M.norm(x[i]));
                if (v != ~0u) {
                    out[i] = r[i] + v, act[k] = act[--na];
                    continue;
                }
                x[i] = M.mul(x[i], step), r[i] += B, ++k;
            }
    }
};

template <u32 K1, u32 SM, bool PF = false, u32 PD = 8, bool SEG = false, bool W5 = false, bool M6 = false, bool M30 = false, bool M6U = false>
struct DlogFixed {
    u32 p, pm1, halfp;
    bool small;
    u64 bk = 0;  // floor(2^64 NB / p)
    Zeroed dmem, fmem;
    u32 *dl, *fa;
    u32 addm(u32 a, u32 b) const {  // (a + b) mod (p - 1)、a, b < p - 1
        const u32 t = a + b - pm1;
        return t + (pm1 & (u32)((i32)t >> 31));
    }
    u32 negm(u32 a) const { return a ? pm1 - a : 0; }
    u32 bucket(u32 a) const { return (u32)((u128)a * bk >> 64); }
    DlogFixed(u32 p_, u32 g) : p(p_), pm1(p_ - 1), halfp(p_ / 2), small(p_ < M0), dmem(4 * (size_t)(small ? p_ : LIM + 1)), fmem(small ? 0 : 4 * (size_t)NB) {
        dl = (u32*)dmem.get(), fa = (u32*)fmem.get();
        build(g);
    }
    void build(u32 g) {
        // メンバを局所変数に写す (log の表への書き込みと別名にならない)
        const u32 p = this->p, pm1 = this->pm1, halfp = this->halfp;
        u32* const dl = this->dl;
        u32* const fa = this->fa;
        auto addm = [pm1](u32 a, u32 b) {
            const u32 t = a + b - pm1;
            return t + (pm1 & (u32)((i32)t >> 31));
        };
        auto negm = [pm1](u32 a) { return a ? pm1 - a : 0; };
        const Mont M(p);
        if (small) {
            const u32 gm = M.to(g);
            u32 x = gm;
            for (u32 i = 1; i < pm1; ++i, x = M.mul(x, gm)) dl[M.from(x)] = i;
            return;
        }
        const u64 bk = this->bk = (u64)(((u128)NB << 64) / p);
        auto bucket = [bk](u32 a) { return (u32)((u128)a * bk >> 64); };
        // Farey の表。区画 [x, y) を f1 と f2 の間の分数で埋める (分数は分子 << 16 | 分母)
        auto rec = [&](auto& self, u32 f1, u32 f2, u32 x, u32 y) -> void {
            const u32 f3 = f1 + f2;
            u32 l = (u32)((((u64)p * (f3 >> 16) - LIM) * NB - 1) / ((u64)p * (f3 & 0xffff)) + 1);
            u32 r = (u32)((((u64)p * (f3 >> 16) + LIM) * NB) / ((u64)p * (f3 & 0xffff)));
            l = std::max(l, x), r = std::min(r, y);
            if (x < l) self(self, f1, f3, x, l);
            for (u32 i = l; i < r; ++i) fa[i] = f3;
            if (r < y) self(self, f3, f2, r, y);
        };
        {
            const u32 fx = (u32)((u64)LIM * NB / p), fy = (u32)(((u64)(p - LIM) * NB - 1) / (p * 2ull) + 1);
            for (u32 i = 0; i < fx; ++i) fa[i] = 1;
            rec(rec, 1u, 0x10002u, fx, fy);
            for (u32 i = fy; i < NB / 2; ++i) fa[i] = 0x10002u;
            for (u32 i = NB / 2; i < NB; ++i) fa[i] = (fa[NB - 1 - i] * 0xffff0001u ^ 0xffff0000u) + 0x10000u;
        }
        // SQ までの最小の素因数
        std::vector<u32> spf(SQ + 1, 0), primes;
        primes.reserve(3500);
        for (u32 i = 2; i <= SQ; ++i) {
            if (!spf[i]) spf[i] = i, primes.push_back(i);
            for (u32 q : primes) {
                if (q * i > SQ) break;
                spf[q * i] = q;
                if (q == spf[i]) break;
            }
        }
        // 1 段目: 最初の K1 個の素数の log を Pohlig-Hellman で
        {
            std::vector<std::pair<u32, u32>> fac;
            u32 m = pm1;
            for (u32 q : primes) {
                if ((u64)q * q > m) break;
                if (m % q == 0) {
                    u32 e = 0;
                    do ++e, m /= q;
                    while (m % q == 0);
                    fac.emplace_back(q, e);
                }
            }
            if (m > 1) fac.emplace_back(m, 1);
            const u32 ginv = inv_mod(g, p);
            std::array<u32, K1> ans{};
            u32 mod = 1;
            for (auto [q, e] : fac) {
                u32 qe = 1;
                for (u32 i = 0; i < e; ++i) qe *= q;
                const u32 ee = pm1 / qe;
                const u32 gg = M.powi(ginv, ee);  // 位数 qe の部分群の生成元の逆元
                const Bsgs bs(M, M.powi(g, pm1 / q), q, q < 100 ? q : (u32)std::sqrt((double)q * K1 / 2));
                const u32 inv_mod_q = inv_mod(mod % qe, qe);
                std::array<u32, K1> h, x, c, d;
                for (u32 i = 0; i < K1; ++i) h[i] = M.powi(primes[i], ee), x[i] = 0;
                u32 pe = qe, mul = 1;
                for (u32 j = 0; j < e; ++j) {  // q 進の桁を下から。桁ごとに K1 個を並べて引く
                    pe /= q;
                    for (u32 i = 0; i < K1; ++i) c[i] = M.powi((u32)((u64)h[i] * M.powi(gg, x[i]) % p), pe);
                    bs.solve(c.data(), K1, d.data());
                    for (u32 i = 0; i < K1; ++i) x[i] += d[i] * mul;
                    mul *= q;
                }
                for (u32 i = 0; i < K1; ++i) {
                    // ans[i] ≡ ans[i] (mod mod)、≡ x (mod qe) をまとめる
                    const u32 k = (u32)((u64)((x[i] + qe - ans[i] % qe) % qe) * inv_mod_q % qe);
                    ans[i] += mod * k;
                }
                mod *= qe;
            }
            for (u32 i = 0; i < K1; ++i) dl[primes[i]] = ans[i];
        }
        // 2 段目: SQ までの残りの素数と、SM 以下の素数の冪を掛けた smooth な数
        {
            const u32 cinv = M.from(M.pow(M.to(inv_mod(g, p)), WK)), cinvm = M.to(cinv), wk = WK % pm1;
            std::vector<u32> smooth{1};
            smooth.reserve(5000);
            for (u32 q : primes) {
                if (!dl[q]) {
                    u32 h = q, t = 0;
                    for (;; ++t, h = M.norm(M.mul(h, cinvm))) {  // h c^{-1} (h は普通の形、cinvm は Montgomery 形)
                        const u32 f = fa[bucket(h)], den = f & 0xffff;
                        const i32 num = (i32)(den * h - (f >> 16) * p);
                        const u32 an = (u32)std::abs(num);
                        if ((den != 1 && !dl[den]) || (an != 1 && !dl[an])) continue;
                        dl[q] = addm(addm(addm(dl[an], num < 0 ? halfp : 0), negm(dl[den])), (u32)((u64)t * wk % pm1));
                        break;
                    }
                }
                if (q > SM) continue;
                smooth.erase(std::remove_if(smooth.begin(), smooth.end(), [&](u32 v) { return v > LIM / q; }), smooth.end());
                const size_t nn = smooth.size();
                for (size_t j = 0; j < nn; ++j) {
                    u32 prev = smooth[j], ns = prev * q;
                    do {
                        if (ns <= LIM / q) smooth.push_back(ns);
                        dl[ns] = addm(dl[prev], dl[q]);
                        prev = ns, ns *= q;
                    } while (prev <= LIM / q);
                }
            }
        }
        // 3 段目: SQ までの合成数
        for (u32 i = 2; i <= SQ; ++i)
            if (!dl[i]) dl[i] = addm(dl[i / spf[i]], dl[spf[i]]);
        // 5 段目: SQ より大きい i は p = q i + r (r = p mod i < i、q = p / i < i) から log i = log r + log(-1) - log q。
        // i が 1 増えると q は高々 1 減る (i (i + 1) > p)
        u32 i = SQ + 1, q = p / i, r = p - q * i;
        if constexpr (SEG) {
            constexpr u32 S = 1u << 15, RL = 1141;  // RL^2 > LIM
            for (u32 lo = SQ + 1; lo <= LIM;) {
                const u32 hi = std::min(LIM + 1, lo + std::min(S, lo));
                for (u32 a : primes) {
                    if (a >= RL) break;
                    const u32 la = dl[a];
                    u32 m = (lo + a - 1) / a;
                    for (u32 j = m * a; j < hi; j += a, ++m) dl[j] = addm(la, dl[m]);
                }
                for (; i < hi; ++i) {
                    if (!dl[i]) dl[i] = addm(addm(dl[r], halfp), negm(dl[q]));
                    if (r >= q) r -= q;
                    else r += i + 1 - q, --q;
                }
                lo = hi;
            }
        } else if constexpr (M6U) {
            const u32 l2 = dl[2], l3 = dl[3];
            auto adv = [&](u32 j) {  // (q, r) を j から j + 1 へ
                if (r >= q) r -= q;
                else r += j + 1 - q, --q;
            };
            auto one = [&](u32 j) {
                if (j % 2 == 0) dl[j] = addm(l2, dl[j >> 1]);
                else if (j % 3 == 0) dl[j] = addm(l3, dl[j / 3]);
                else dl[j] = addm(addm(dl[r], halfp), negm(dl[q]));
                adv(j);
            };
            for (; i % 6 && i <= LIM; ++i) one(i);
            for (; i + 5 <= LIM; i += 6) {
                const u32 h = i >> 1, t = i / 3;
                dl[i] = addm(l2, dl[h]), adv(i);
                dl[i + 1] = addm(addm(dl[r], halfp), negm(dl[q])), adv(i + 1);
                dl[i + 2] = addm(l2, dl[h + 1]), adv(i + 2);
                dl[i + 3] = addm(l3, dl[t + 1]), adv(i + 3);
                dl[i + 4] = addm(l2, dl[h + 2]), adv(i + 4);
                dl[i + 5] = addm(addm(dl[r], halfp), negm(dl[q])), adv(i + 5);
            }
            for (; i <= LIM; ++i) one(i);
        } else if constexpr (M30) {
            // 種類: 0 は 2 で、1 は 3 で、2 は 5 で割り切れる (小さい素数を先に)、3 は 30 と互いに素
            static constexpr std::array<u8, 30> TY = [] {
                std::array<u8, 30> t{};
                for (int c = 0; c < 30; ++c) t[c] = c % 2 == 0 ? 0 : c % 3 == 0 ? 1 : c % 5 == 0 ? 2 : 3;
                return t;
            }();
            const u32 l2 = dl[2], l3 = dl[3], l5 = dl[5];
            u32 c = i % 30;
            for (; i <= LIM; ++i) {
                const u32 t = TY[c];
                if (t == 0) dl[i] = addm(l2, dl[i >> 1]);
                else if (t == 1) dl[i] = addm(l3, dl[(u32)((u64)i * 0xAAAAAAABu >> 33)]);
                else if (t == 2) dl[i] = addm(l5, dl[(u32)((u64)i * 0xCCCCCCCDu >> 34)]);
                else dl[i] = addm(addm(dl[r], halfp), negm(dl[q]));
                c = c == 29 ? 0 : c + 1;
                if (r >= q) r -= q;
                else r += i + 1 - q, --q;
            }
        } else if constexpr (M6) {
            const u32 l2 = dl[2], l3 = dl[3];
            u32 c6 = i % 6;
            for (; i <= LIM; ++i) {
                if (c6 % 2 == 0) dl[i] = addm(l2, dl[i >> 1]);
                else if (c6 == 3) dl[i] = addm(l3, dl[(u32)((u64)i * 0xAAAAAAABu >> 33)]);
                else dl[i] = addm(addm(dl[r], halfp), negm(dl[q]));
                c6 = c6 == 5 ? 0 : c6 + 1;
                if (r >= q) r -= q;
                else r += i + 1 - q, --q;
            }
        } else {
            for (; i <= LIM; ++i) {
                if constexpr (PF)
                    if (r >= PD * q) __builtin_prefetch(dl + r - PD * q);
                if (W5 || !dl[i]) dl[i] = addm(addm(dl[r], halfp), negm(dl[q]));
                if (r >= q) r -= q;
                else r += i + 1 - q, --q;
            }
        }
    }
    struct View {
        const u32 *dl, *fa;
        u64 bk;
        u32 p, pm1, halfp;
        bool small;
        u32 operator()(u32 a) const {
            if (small) return a ? dl[a] : ~0u;
            const u32 f = fa[(u32)((u128)a * bk >> 64)], den = f & 0xffff;
            const i32 num = (i32)(den * a - (f >> 16) * p);
            const u32 s = (u32)(num >> 31), an = ((u32)num ^ s) - s;
            u32 r = dl[an] + (halfp & s) + pm1 - dl[den];  // < 3 (p - 1)
            u32 t = r - pm1;
            r = t + (pm1 & (u32)((i32)t >> 31));
            t = r - pm1;
            r = t + (pm1 & (u32)((i32)t >> 31));
            return a ? r : ~0u;
        }
    };
    View view() const { return {dl, fa, bk, p, pm1, halfp, small}; }
    u32 operator()(u32 a) const { return view()(a); }
};

}  // namespace dlf5
