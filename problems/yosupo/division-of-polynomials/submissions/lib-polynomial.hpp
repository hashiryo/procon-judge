#pragma once
#include "common.hpp"
#include "mylib/fft/Polynomial.hpp"

// mylib の Polynomial の quorem (素朴な筆算と NTT を大きさで選ぶ) で商と余りを作る。
struct Solver {
  Polynomial<Mint> f, g, q, r;

  Solver(const vector<i64> &src_f, const vector<i64> &src_g)
      : f(to_mint(src_f)), g(to_mint(src_g)) {}

  void run() { auto qr = f.quorem(g); q = qr.first, r = qr.second; }

  vector<i64> quotient() const { return coeffs(q); }
  vector<i64> remainder() const { return coeffs(r); }

 private:
  static vector<i64> coeffs(const Polynomial<Mint> &p) {
    const int d = p.deg() + 1;
    vector<i64> v(d);
    for (int i = 0; i < d; ++i) v[i] = p[i].val();
    return v;
  }
};
