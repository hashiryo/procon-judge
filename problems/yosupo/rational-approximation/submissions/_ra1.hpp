#pragma once
// 有理数近似の 1 回目。x/y への Stern-Brocot 木の道を、x/y の連分数展開 (互除法の商) の連ごとに進める。端 L ≤ x/y ≤ R
// (初めは 0/1 と 1/0) を持ち、今いる節点は L + R。R の連 t 歩では L += t R、L の連では R += t L で、最後の連は商 - 1 歩
// 進んだ節点が x/y そのもの (x/y が既約でなくても商の列は同じ)。木を下るのは今いる節点の分子と分母が N 以下の間だけで、
// R の連で s 歩進んだ節点 L + (s + 1) R が N を超えたら、そこで (L + s R, R) が答え。連を全部進めても N 以下かは
// 掛け算と比べだけで分かるので、超える連でだけ s = min(⌊(N - L_x) / R_x⌋, ⌊(N - L_y) / R_y⌋) を割り算で出す。
// 端は u64 で持ち、(t + 1) R が 10^18 程度になっても溢れない。
#include "../common.hpp"

namespace ra1 {

inline void solve(u32 n, u32 x, u32 y, u32* out) {
    u64 lx = 0, ly = 1, rx = 1, ry = 0;
    u32 a = x, b = y;
    while (true) {
        // R の連
        {
            const u32 q = a / b, r = a % b;
            const u64 t = q - (r == 0);
            if (lx + (t + 1) * rx <= n && ly + (t + 1) * ry <= n) {
                lx += t * rx, ly += t * ry;
                if (r == 0) {
                    out[0] = out[2] = (u32)(lx + rx), out[1] = out[3] = (u32)(ly + ry);
                    return;
                }
            } else {
                u64 c = (n - lx) / rx;
                if (ry) c = std::min(c, (n - ly) / ry);
                lx += c * rx, ly += c * ry;
                out[0] = (u32)lx, out[1] = (u32)ly, out[2] = (u32)rx, out[3] = (u32)ry;
                return;
            }
            a = b, b = r;
        }
        // L の連
        {
            const u32 q = a / b, r = a % b;
            const u64 t = q - (r == 0);
            if (rx + (t + 1) * lx <= n && ry + (t + 1) * ly <= n) {
                rx += t * lx, ry += t * ly;
                if (r == 0) {
                    out[0] = out[2] = (u32)(lx + rx), out[1] = out[3] = (u32)(ly + ry);
                    return;
                }
            } else {
                u64 c = (n - ry) / ly;
                if (lx) c = std::min(c, (n - rx) / lx);
                rx += c * lx, ry += c * ly;
                out[0] = (u32)lx, out[1] = (u32)ly, out[2] = (u32)rx, out[3] = (u32)ry;
                return;
            }
            a = b, b = r;
        }
    }
}

inline void run(const std::vector<Query>& qs, u32* out) {
    for (const auto& q : qs) solve(q.n, q.x, q.y, out), out += 4;
}

}  // namespace ra1
