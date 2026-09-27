// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

#include <iostream>
#include <vector>
#include <type_traits>
#include <numeric>
#include <vector>
#include <cassert>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <valarray>
#include <cassert>
namespace _la_internal {
using namespace std;
template <class R> struct Vector {
 valarray<R> dat;
 Vector()= default;
 Vector(size_t n): dat(n) {}
 Vector(size_t n, const R &v): dat(v, n) {}
 Vector(const initializer_list<R> &v): dat(v) {}
 R &operator[](int i) { return dat[i]; }
 const R &operator[](int i) const { return dat[i]; }
 bool operator==(const Vector &r) const {
  if (dat.size() != r.dat.size()) return false;
  for (int i= dat.size(); i--;)
   if (dat[i] != r.dat[i]) return false;
  return true;
 }
 bool operator!=(const Vector &r) const { return !(*this == r); }
 explicit operator bool() const { return dat.size(); }
 Vector operator-() const { return Vector(dat.size())-= *this; }
 Vector &operator+=(const Vector &r) { return dat+= r.dat, *this; }
 Vector &operator-=(const Vector &r) { return dat-= r.dat, *this; }
 Vector &operator*=(const R &r) { return dat*= r, *this; }
 Vector operator+(const Vector &r) const { return Vector(*this)+= r; }
 Vector operator-(const Vector &r) const { return Vector(*this)-= r; }
 Vector operator*(const R &r) const { return Vector(*this)*= r; }
 size_t size() const { return dat.size(); }
 friend R dot(const Vector<R> &a, const Vector<R> &b) { return assert(a.size() == b.size()), (a.dat * b.dat).sum(); }
};
using u128= __uint128_t;
using u64= uint64_t;
using u8= uint8_t;
class Ref {
 u128 *ref;
 u8 i;
public:
 Ref(u128 *ref, u8 i): ref(ref), i(i) {}
 Ref &operator=(const Ref &r) { return *this= bool(r); }
 Ref &operator=(bool b) { return *ref&= ~(u128(1) << i), *ref|= u128(b) << i, *this; }
 Ref &operator|=(bool b) { return *ref|= u128(b) << i, *this; }
 Ref &operator&=(bool b) { return *ref&= ~(u128(!b) << i), *this; }
 Ref &operator^=(bool b) { return *ref^= u128(b) << i, *this; }
 operator bool() const { return (*ref >> i) & 1; }
};
template <> class Vector<bool> {
 size_t n;
public:
 valarray<u128> dat;
 Vector(): n(0) {}
 Vector(size_t n): n(n), dat((n + 127) >> 7) {}
 Vector(size_t n, bool b): n(n), dat(-u128(b), (n + 127) >> 7) {
  if (int k= n & 127; k) dat[dat.size() - 1]&= (u128(1) << k) - 1;
 }
 Vector(const initializer_list<bool> &v): n(v.size()), dat((n + 127) >> 7) {
  int i= 0;
  for (bool b: v) dat[i >> 7]|= u128(b) << (i & 127), ++i;
 }
 Ref operator[](int i) { return {begin(dat) + (i >> 7), u8(i & 127)}; }
 bool operator[](int i) const { return (dat[i >> 7] >> (i & 127)) & 1; }
 bool operator==(const Vector &r) const {
  if (dat.size() != r.dat.size()) return false;
  for (int i= dat.size(); i--;)
   if (dat[i] != r.dat[i]) return false;
  return true;
 }
 bool operator!=(const Vector &r) const { return !(*this == r); }
 explicit operator bool() const { return n; }
 Vector operator-() const { return Vector(*this); }
 Vector &operator+=(const Vector &r) { return dat^= r.dat, *this; }
 Vector &operator-=(const Vector &r) { return dat^= r.dat, *this; }
 Vector &operator*=(bool b) { return dat*= b, *this; }
 Vector operator+(const Vector &r) const { return Vector(*this)+= r; }
 Vector operator-(const Vector &r) const { return Vector(*this)-= r; }
 Vector operator*(bool b) const { return Vector(*this)*= b; }
 size_t size() const { return n; }
 friend bool dot(const Vector<bool> &a, const Vector<bool> &b) {
  assert(a.size() == b.size());
  u128 v= 0;
  for (int i= a.dat.size(); i--;) v^= a.dat[i] & b.dat[i];
  return __builtin_parityll(v >> 64) ^ __builtin_parityll(u64(v));
 }
};
template <class R> Vector<R> operator*(const R &r, const Vector<R> &v) { return v * r; }
template <class R> ostream &operator<<(ostream &os, const Vector<R> &v) {
 os << '[';
 for (int _= 0, __= v.size(); _ < __; ++_) os << (_ ? ", " : "") << v[_];
 return os << ']';
}
}
using _la_internal::Vector;
namespace _la_internal {
template <class R, class D> struct Mat {
 Mat(): W(0) {}
 Mat(size_t h, size_t w): W(w), dat(h * w) {}
 Mat(size_t h, size_t w, R v): W(w), dat(v, h * w) {}
 Mat(initializer_list<initializer_list<R>> v): W(v.size() ? v.begin()->size() : 0), dat(v.size() * W) {
  auto it= begin(dat);
  for(const auto& r: v) {
   assert(r.size() == W);
   for(R x: r) *it++= x;
  }
 }
 size_t width() const { return W; }
 size_t height() const { return W ? dat.size() / W : 0; }
 auto operator[](int i) { return begin(dat) + i * W; }
 auto operator[](int i) const { return begin(dat) + i * W; }
protected:
 size_t W;
 valarray<R> dat;
 void add(const Mat& r) { assert(dat.size() == r.dat.size()), assert(W == r.W), dat+= r.dat; }
 D mul(const Mat& r) const {
  const size_t h= height(), w= r.W, l= W;
  assert(l == r.height());
  D ret(h, w);
  auto a= begin(dat);
  auto c= begin(ret.dat);
  for(int i= h; i--; c+= w) {
   auto b= begin(r.dat);
   for(int k= l; k--; ++a) {
    auto d= c;
    auto v= *a;
    for(int j= w; j--; ++b, ++d) *d+= v * *b;
   }
  }
  return ret;
 }
 Vector<R> mul(const Vector<R>& r) const {
  assert(W == r.size());
  const size_t h= height();
  Vector<R> ret(h);
  auto a= begin(dat);
  for(size_t i= 0; i < h; ++i)
   for(size_t k= 0; k < W; ++k, ++a) ret[i]+= *a * r[k];
  return ret;
 }
};
template <class D> struct Mat<bool, D> {
 struct Array {
  u128* bg;
  Array(u128* it): bg(it) {}
  Ref operator[](int i) { return Ref{bg + (i >> 7), u8(i & 127)}; }
  bool operator[](int i) const { return (bg[i >> 7] >> (i & 127)) & 1; }
 };
 struct ConstArray {
  const u128* bg;
  ConstArray(const u128* it): bg(it) {}
  bool operator[](int i) const { return (bg[i >> 7] >> (i & 127)) & 1; }
 };
 Mat(): H(0), W(0), m(0) {}
 Mat(size_t h, size_t w): H(h), W(w), m((w + 127) >> 7), dat(h * m) {}
 Mat(size_t h, size_t w, bool b): H(h), W(w), m((w + 127) >> 7), dat(-u128(b), h * m) {
  if(size_t i= h, k= w & 127; k)
   for(u128 s= (u128(1) << k) - 1; i--;) dat[i * m]&= s;
 }
 Mat(const initializer_list<initializer_list<bool>>& v): H(v.size()), W(H ? v.begin()->size() : 0), m((W + 127) >> 7), dat(H * m) {
  auto it= begin(dat);
  for(const auto& r: v) {
   assert(r.size() == W);
   int i= 0;
   for(bool b: r) it[i >> 7]|= u128(b) << (i & 127), ++i;
   it+= m;
  }
 }
 size_t width() const { return W; }
 size_t height() const { return H; }
 Array operator[](int i) { return {begin(dat) + i * m}; }
 ConstArray operator[](int i) const { return {begin(dat) + i * m}; }
 ConstArray get(int i) const { return {begin(dat) + i * m}; }
protected:
 size_t H, W, m;
 valarray<u128> dat;
 void add(const Mat& r) { assert(H == r.H), assert(W == r.W), dat^= r.dat; }
 D mul(const Mat& r) const {
  assert(W == r.H);
  D ret(H, r.W);
  valarray<u128> tmp(r.m << 8);
  auto y= begin(r.dat);
  for(size_t l= 0; l < W; l+= 8) {
   auto t= begin(tmp) + r.m;
   for(int i= 0, n= min<size_t>(8, W - l); i < n; ++i, y+= r.m) {
    auto u= begin(tmp);
    for(int s= 1 << i; s--;) {
     auto z= y;
     for(int j= r.m; j--; ++u, ++t, ++z) *t= *u ^ *z;
    }
   }
   auto a= begin(dat) + (l >> 7);
   auto c= begin(ret.dat);
   for(int i= H; i--; a+= m) {
    auto u= begin(tmp) + ((*a >> (l & 127)) & 255) * r.m;
    for(int j= r.m; j--; ++c, ++u) *c^= *u;
   }
  }
  return ret;
 }
 Vector<bool> mul(const Vector<bool>& r) const {
  assert(W == r.size());
  Vector<bool> ret(H);
  auto a= begin(dat);
  for(size_t i= 0; i < H; ++i) {
   u128 v= 0;
   for(size_t j= 0; j < m; ++j, ++a) v^= *a & r.dat[j];
   ret[i]= __builtin_parityll(v >> 64) ^ __builtin_parityll(u64(v));
  }
  return ret;
 }
};
template <class R> struct Matrix: public Mat<R, Matrix<R>> {
 using Mat<R, Matrix<R>>::Mat;
 explicit operator bool() const { return this->W; }
 static Matrix identity(int n) {
  Matrix ret(n, n);
  for(; n--;) ret[n][n]= R(true);
  return ret;
 }
 Matrix submatrix(const vector<int>& rows, const vector<int>& cols) const {
  Matrix ret(rows.size(), cols.size());
  for(int i= rows.size(); i--;)
   for(int j= cols.size(); j--;) ret[i][j]= (*this)[rows[i]][cols[j]];
  return ret;
 }
 Matrix submatrix_rm(vector<int> rows, vector<int> cols) const {
  sort(begin(rows), end(rows)), sort(begin(cols), end(cols)), rows.erase(unique(begin(rows), end(rows)), end(rows)), cols.erase(unique(begin(cols), end(cols)), end(cols));
  const int H= this->height(), W= this->width(), n= rows.size(), m= cols.size();
  vector<int> rs(H - n), cs(W - m);
  for(int i= 0, j= 0, k= 0; i < H; ++i)
   if(j < n && rows[j] == i) ++j;
   else rs[k++]= i;
  for(int i= 0, j= 0, k= 0; i < W; ++i)
   if(j < m && cols[j] == i) ++j;
   else cs[k++]= i;
  return submatrix(rs, cs);
 }
 bool operator==(const Matrix& r) const {
  if(this->width() != r.width() || this->height() != r.height()) return false;
  for(int i= this->dat.size(); i--;)
   if(this->dat[i] != r.dat[i]) return false;
  return true;
 }
 bool operator!=(const Matrix& r) const { return !(*this == r); }
 Matrix& operator*=(const Matrix& r) { return *this= this->mul(r); }
 Matrix operator*(const Matrix& r) const { return this->mul(r); }
 Matrix& operator*=(R r) { return this->dat*= r, *this; }
 template <class T> Matrix operator*(T r) const {
  static_assert(is_convertible_v<T, R>);
  return Matrix(*this)*= r;
 }
 Matrix& operator+=(const Matrix& r) { return this->add(r), *this; }
 Matrix operator+(const Matrix& r) const { return Matrix(*this)+= r; }
 Vector<R> operator*(const Vector<R>& r) const { return this->mul(r); }
 Vector<R> operator()(const Vector<R>& r) const { return this->mul(r); }
 Matrix pow(uint64_t k) const {
  size_t W= this->width();
  assert(W == this->height());
  for(Matrix ret= identity(W), b= *this;; b*= b)
   if(k& 1 ? ret*= b, !(k>>= 1) : !(k>>= 1)) return ret;
 }
};
template <class R, class T> Matrix<R> operator*(const T& r, const Matrix<R>& m) { return m * r; }
template <class R> ostream& operator<<(ostream& os, const Matrix<R>& m) {
 os << "\n[";
 for(int i= 0, h= m.height(); i < h; os << ']', ++i) {
  if(i) os << "\n ";
  os << '[';
  for(int j= 0, w= m.width(); j < w; ++j) os << (j ? ", " : "") << m[i][j];
 }
 return os << ']';
}
template <class K> static bool is_zero(K x) {
 if constexpr(is_floating_point_v<K>) return abs(x) < 1e-8;
 else return x == K();
}
}
using _la_internal::Matrix;

namespace _la_internal {
template <class K> class LU_Decomposition {
 Matrix<K> dat;
 vector<size_t> perm, piv;
 bool sgn;
 size_t psz;
public:
 LU_Decomposition(const Matrix<K>& A): dat(A), perm(A.height()), sgn(false), psz(0) {
  const size_t h= A.height(), w= A.width();
  iota(perm.begin(), perm.end(), 0), piv.resize(min(w, h));
  for(size_t c= 0, pos; c < w && psz < h; ++c) {
   pos= psz;
   if constexpr(is_floating_point_v<K>) {
    for(size_t r= psz + 1; r < h; ++r)
     if(abs(dat[perm[pos]][c]) < abs(dat[perm[r]][c])) pos= r;
   } else if(is_zero(dat[perm[pos]][c]))
    for(size_t r= psz + 1; r < h; ++r)
     if(!is_zero(dat[perm[r]][c])) pos= r, r= h;
   if(is_zero(dat[perm[pos]][c])) continue;
   if(pos != psz) sgn= !sgn, swap(perm[pos], perm[psz]);
   const auto b= dat[perm[psz]];
   for(size_t r= psz + 1, i; r < h; ++r) {
    auto a= dat[perm[r]];
    K m= a[c] / b[c];
    for(a[c]= K(), a[psz]= m, i= c + 1; i < w; ++i) a[i]-= b[i] * m;
   }
   piv[psz++]= c;
  }
 }
 size_t rank() const { return psz; }
 bool is_regular() const { return rank() == dat.height() && rank() == dat.width(); }
 K det() const {
  assert(dat.height() == dat.width());
  K ret= sgn ? -1 : 1;
  for(size_t i= dat.width(); i--;) ret*= dat[perm[i]][i];
  return ret;
 }
 vector<Vector<K>> kernel() const {
  const size_t w= dat.width(), n= rank();
  vector ker(w - n, Vector<K>(w));
  for(size_t c= 0, i= 0; c < w; ++c) {
   if(i < n && piv[i] == c) ++i;
   else {
    auto& a= ker[c - i];
    a[c]= 1;
    for(size_t r= i; r--;) a[r]= -dat[perm[r]][c];
    for(size_t j= i, k, r; j--;) {
     K x= a[j] / dat[perm[j]][k= piv[j]];
     for(a[j]= 0, a[k]= x, r= j; r--;) a[r]-= dat[perm[r]][k] * x;
    }
   }
  }
  return ker;
 }
 Vector<K> linear_equations(const Vector<K>& b) const {
  const size_t h= dat.height(), w= dat.width(), n= rank();
  assert(h == b.size());
  Vector<K> y(h), x(w);
  for(size_t c= 0; c < h; ++c)
   if(y[c]+= b[perm[c]]; c < w)
    for(size_t r= c + 1; r < h; ++r) y[r]-= y[c] * dat[perm[r]][c];
  for(size_t i= n; i < h; ++i)
   if(!is_zero(y[i])) return Vector<K>();  // no solution
  for(size_t i= n, r; i--;)
   for(x[piv[i]]= y[i] / dat[perm[i]][piv[i]], r= i; r--;) y[r]-= x[piv[i]] * dat[perm[r]][piv[i]];
  return x;
 }
 Matrix<K> inverse_matrix() const {
  if(!is_regular()) return Matrix<K>();  // no solution
  const size_t n= dat.width();
  Matrix<K> ret(n, n);
  for(size_t i= 0; i < n; ++i) {
   Vector<K> y(n);
   for(size_t c= 0; c < n; ++c)
    if(y[c]+= perm[c] == i; !is_zero(y[c]))
     for(size_t r= c + 1; r < n; ++r) y[r]-= y[c] * dat[perm[r]][c];
   for(size_t j= n; j--;) {
    K m= ret[j][i]= y[j] / dat[perm[j]][j];
    for(size_t r= j; r--;) y[r]-= m * dat[perm[r]][j];
   }
  }
  return ret;
 }
};
void add_upper(u128* a, const u128* b, size_t bg, size_t ed) {  //[bg,ed)
 if(bg >= ed) return;
 size_t s= bg >> 7;
 a[s]^= b[s] & -(u128(1) << (bg & 127));
 for(size_t i= (ed + 127) >> 7; --i > s;) a[i]^= b[i];
}
void add_lower(u128* a, const u128* b, size_t ed) {  //[0,ed)
 size_t s= ed >> 7;
 for(a[s]^= b[s] & ((u128(1) << (ed & 127)) - 1); s--;) a[s]^= b[s];
}
void subst_lower(u128* a, const u128* b, size_t ed) {  //[0,ed)
 size_t s= ed >> 7;
 for(a[s]= b[s] & ((u128(1) << (ed & 127)) - 1); s--;) a[s]= b[s];
}
bool any1_upper(const u128* a, size_t bg, size_t ed) {  //[bg,ed)
 if(bg >= ed) return false;
 size_t s= bg >> 7;
 if(a[s] & -(u128(1) << (bg & 127))) return true;
 for(size_t i= (ed + 127) >> 7; --i > s;)
  if(a[i]) return true;
 return false;
}
template <> class LU_Decomposition<bool> {
 Matrix<bool> dat;
 vector<size_t> perm, piv;
 size_t psz;
public:
 LU_Decomposition(Matrix<bool> A): dat(A.width(), A.height()), perm(A.height()), psz(0) {
  const size_t h= A.height(), w= A.width();
  iota(perm.begin(), perm.end(), 0), piv.resize(min(w, h));
  for(size_t c= 0, pos; c < w && psz < h; ++c) {
   for(pos= psz; pos < h; ++pos)
    if(A.get(perm[pos])[c]) break;
   if(pos == h) continue;
   if(pos != psz) swap(perm[pos], perm[psz]);
   auto b= A.get(perm[psz]);
   for(size_t r= psz + 1; r < h; ++r) {
    auto a= A[perm[r]];
    if(bool m= a[c]; m) add_upper(a.bg, b.bg, c, w), a[psz]= 1;
   }
   piv[psz++]= c;
  }
  for(size_t j= w; j--;)
   for(size_t i= h; i--;) dat[j][i]= A.get(perm[i])[j];
 }
 size_t rank() const { return psz; }
 bool is_regular() const { return rank() == dat.height() && rank() == dat.width(); }
 bool det() const { return is_regular(); }
 vector<Vector<bool>> kernel() const {
  const size_t w= dat.height(), n= rank();
  vector ker(w - rank(), Vector<bool>(w));
  for(size_t c= 0, i= 0; c < w; ++c) {
   if(i < n && piv[i] == c) ++i;
   else {
    auto& a= ker[c - i];
    subst_lower(begin(a.dat), dat[c].bg, i), a[c]= 1;
    for(size_t j= i, k; j--;) {
     bool x= a[j];
     if(a[j]= 0, a[k= piv[j]]= x; x) add_lower(begin(a.dat), dat[k].bg, j);
    }
   }
  }
  return ker;
 }
 Vector<bool> linear_equations(const Vector<bool>& b) const {
  const size_t h= dat.width(), w= dat.height(), n= rank();
  assert(h == b.size());
  Vector<bool> y(h), x(w);
  for(size_t c= 0; c < h; ++c)
   if(y[c]^= b[perm[c]]; c < w && y[c]) add_upper(begin(y.dat), dat[c].bg, c + 1, h);
  if(any1_upper(begin(y.dat), n, h)) return Vector<bool>();  // no solution
  for(size_t i= n; i--;)
   if((x[piv[i]]= y[i])) add_lower(begin(y.dat), dat[piv[i]].bg, i);
  return x;
 }
 Matrix<bool> inverse_matrix() const {
  if(!is_regular()) return Matrix<bool>();  // no solution
  const size_t n= dat.width();
  Matrix<bool> ret(n, n);
  for(size_t i= 0; i < n; ++i) {
   Vector<bool> y(n);
   for(size_t c= 0; c < n; ++c)
    if(y[c]^= perm[c] == i; y[c]) add_upper(begin(y.dat), dat[c].bg, c + 1, n);
   for(size_t j= n; j--;)
    if((ret[j][i]= y[j])) add_lower(begin(y.dat), dat[j].bg, j);
  }
  return ret;
 }
};
}
using _la_internal::LU_Decomposition;

#include <iostream>
#include <utility>
#include <type_traits>
#include <cassert>
template <class Uint> constexpr inline Uint mod_inv(Uint a, Uint mod) {
 std::make_signed_t<Uint> x= 1, y= 0, z= 0;
 for (Uint q= 0, b= mod, c= 0; b;) z= x, x= y, y= z - y * (q= a / b), c= a, a= b, b= c - b * q;
 return assert(a == 1), x < 0 ? mod - (-x) % mod : x % mod;
}
namespace math_internal {
using namespace std;
using u8= unsigned char;
using u32= unsigned;
using i64= long long;
using u64= unsigned long long;
using u128= __uint128_t;
struct MP_Na {  // mod < 2^32
 u32 mod;
 constexpr MP_Na(): mod(0) {}
 constexpr MP_Na(u32 m): mod(m) {}
 constexpr inline u32 mul(u32 l, u32 r) const { return u64(l) * r % mod; }
 constexpr inline u32 set(u32 n) const { return n; }
 constexpr inline u32 get(u32 n) const { return n; }
 constexpr inline u32 norm(u32 n) const { return n; }
 constexpr inline u32 plus(u64 l, u32 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u32 diff(u64 l, u32 r) const { return l-= r, l >> 63 ? l + mod : l; }
};
template <class u_t, class du_t, u8 B> struct MP_Mo {  // mod < 2^32, mod < 2^62
 u_t mod;
 constexpr MP_Mo(): mod(0), iv(0), r2(0) {}
 constexpr MP_Mo(u_t m): mod(m), iv(inv(m)), r2(-du_t(mod) % mod) {}
 constexpr inline u_t mul(u_t l, u_t r) const { return reduce(du_t(l) * r); }
 constexpr inline u_t set(u_t n) const { return mul(n, r2); }
 constexpr inline u_t get(u_t n) const { return n= reduce(n), n >= mod ? n - mod : n; }
 constexpr inline u_t norm(u_t n) const { return n >= mod ? n - mod : n; }
 constexpr inline u_t plus(u_t l, u_t r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u_t diff(u_t l, u_t r) const { return l-= r, l >> (B - 1) ? l + (mod << 1) : l; }
private:
 u_t iv, r2;
 static constexpr u_t inv(u_t n, int e= 6, u_t x= 1) { return e ? inv(n, e - 1, x * (2 - x * n)) : x; }
 constexpr inline u_t reduce(const du_t &w) const { return u_t(w >> B) + mod - ((du_t(u_t(w) * iv) * mod) >> B); }
};
using MP_Mo32= MP_Mo<u32, u64, 32>;
using MP_Mo64= MP_Mo<u64, u128, 64>;
struct MP_Br {  // 2^20 < mod <= 2^41
 u64 mod;
 constexpr MP_Br(): mod(0), x(0) {}
 constexpr MP_Br(u64 m): mod(m), x((u128(1) << 84) / m) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem(u128(l) * r); }
 static constexpr inline u64 set(u64 n) { return n; }
 constexpr inline u64 get(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 norm(u64 n) const { return n >= mod ? n - mod : n; }
 constexpr inline u64 plus(u64 l, u64 r) const { return l+= r, l < (mod << 1) ? l : l - (mod << 1); }
 constexpr inline u64 diff(u64 l, u64 r) const { return l-= r, l >> 63 ? l + (mod << 1) : l; }
private:
 u64 x;
 constexpr inline u128 quo(const u128 &n) const { return (n * x) >> 84; }
 constexpr inline u64 rem(const u128 &n) const { return n - quo(n) * mod; }
};
template <class du_t, u8 B> struct MP_D2B1 {  // mod < 2^63, mod < 2^64
 u64 mod;
 constexpr MP_D2B1(): mod(0), s(0), d(0), v(0) {}
 constexpr MP_D2B1(u64 m): mod(m), s(__builtin_clzll(m)), d(m << s), v(u128(-1) / d) {}
 constexpr inline u64 mul(u64 l, u64 r) const { return rem((u128(l) * r) << s) >> s; }
 constexpr inline u64 set(u64 n) const { return n; }
 constexpr inline u64 get(u64 n) const { return n; }
 constexpr inline u64 norm(u64 n) const { return n; }
 constexpr inline u64 plus(du_t l, u64 r) const { return l+= r, l < mod ? l : l - mod; }
 constexpr inline u64 diff(du_t l, u64 r) const { return l-= r, l >> B ? l + mod : l; }
private:
 u8 s;
 u64 d, v;
 constexpr inline u64 rem(const u128 &u) const {
  u128 q= (u >> 64) * v + u;
  u64 r= u64(u) - (q >> 64) * d - d;
  if (r > u64(q)) r+= d;
  if (r >= d) r-= d;
  return r;
 }
};
using MP_D2B1_1= MP_D2B1<u64, 63>;
using MP_D2B1_2= MP_D2B1<u128, 127>;
template <class u_t, class MP> constexpr u_t pow(u_t x, u64 k, const MP &md) {
 for (u_t ret= md.set(1);; x= md.mul(x, x))
  if (k & 1 ? ret= md.mul(ret, x) : 0; !(k>>= 1)) return ret;
}
}
#include <type_traits>
namespace math_internal {
struct m_b {};
struct s_b: m_b {};
}
template <class mod_t> constexpr bool is_modint_v= std::is_base_of_v<math_internal::m_b, mod_t>;
template <class mod_t> constexpr bool is_staticmodint_v= std::is_base_of_v<math_internal::s_b, mod_t>;
namespace math_internal {
template <class MP, u64 MOD> struct SB: s_b {
protected:
 static constexpr MP md= MP(MOD);
};
template <class U, class B> struct MInt: public B {
 using Uint= U;
 static constexpr inline auto mod() { return B::md.mod; }
 constexpr MInt(): x(0) {}
 template <class T, typename= enable_if_t<is_modint_v<T> && !is_same_v<T, MInt>>> constexpr MInt(T v): x(B::md.set(v.val() % B::md.mod)) {}
 constexpr MInt(__int128_t n): x(B::md.set((n < 0 ? ((n= (-n) % B::md.mod) ? B::md.mod - n : n) : n % B::md.mod))) {}
 constexpr MInt operator-() const { return MInt() - *this; }
#define FUNC(name, op) \
 constexpr MInt name const { \
  MInt ret; \
  return ret.x= op, ret; \
 }
 FUNC(operator+(const MInt & r), B::md.plus(x, r.x))
 FUNC(operator-(const MInt & r), B::md.diff(x, r.x))
 FUNC(operator*(const MInt & r), B::md.mul(x, r.x))
 FUNC(pow(u64 k), math_internal::pow(x, k, B::md))
#undef FUNC
 constexpr MInt operator/(const MInt& r) const { return *this * r.inv(); }
 constexpr MInt& operator+=(const MInt& r) { return *this= *this + r; }
 constexpr MInt& operator-=(const MInt& r) { return *this= *this - r; }
 constexpr MInt& operator*=(const MInt& r) { return *this= *this * r; }
 constexpr MInt& operator/=(const MInt& r) { return *this= *this / r; }
 constexpr bool operator==(const MInt& r) const { return B::md.norm(x) == B::md.norm(r.x); }
 constexpr bool operator!=(const MInt& r) const { return !(*this == r); }
 constexpr bool operator<(const MInt& r) const { return B::md.norm(x) < B::md.norm(r.x); }
 constexpr inline MInt inv() const { return mod_inv<U>(val(), B::md.mod); }
 constexpr inline Uint val() const { return B::md.get(x); }
 friend ostream& operator<<(ostream& os, const MInt& r) { return os << r.val(); }
 friend istream& operator>>(istream& is, MInt& r) {
  i64 v;
  return is >> v, r= MInt(v), is;
 }
private:
 Uint x;
};
template <u64 MOD> using MP_B= conditional_t < (MOD < (1 << 30)) & MOD, MP_Mo32, conditional_t < MOD < (1ull << 32), MP_Na, conditional_t<(MOD < (1ull << 62)) & MOD, MP_Mo64, conditional_t<MOD<(1ull << 41), MP_Br, conditional_t<MOD<(1ull << 63), MP_D2B1_1, MP_D2B1_2>>>>>;
template <u64 MOD> using ModInt= MInt < conditional_t<MOD<(1 << 30), u32, u64>, SB<MP_B<MOD>, MOD>>;
}
using math_internal::ModInt;

#include <cassert>
#include <vector>
template <class mod_t> class FactorialPrecalculation {
 static_assert(is_modint_v<mod_t>);
 static inline std::vector<mod_t> iv, fct, fiv;
public:
 static void reset() { iv.clear(), fct.clear(), fiv.clear(); }
 static inline mod_t inv(int n) {
  assert(0 < n);
  if(int k= iv.size(); k <= n) {
   if(iv.resize(n + 1); !k) iv[1]= 1, k= 2;
   for(unsigned long long mod= mod_t::mod(), q; k <= n; ++k) q= (mod + k - 1) / k, iv[k]= iv[k * q - mod] * q;
  }
  return iv[n];
 }
 static inline mod_t fact(int n) {
  assert(0 <= n);
  if(int k= fct.size(); k <= n) {
   if(fct.resize(n + 1); !k) fct[0]= 1, k= 1;
   for(; k <= n; ++k) fct[k]= fct[k - 1] * k;
  }
  return fct[n];
 }
 static inline mod_t finv(int n) {
  assert(0 <= n);
  if(int k= fiv.size(); k <= n) {
   if(fiv.resize(n + 1); !k) fiv[0]= 1, k= 1;
   for(; k <= n; ++k) fiv[k]= fiv[k - 1] * inv(k);
  }
  return fiv[n];
 }
 static inline mod_t nPr(int n, int r) { return r < 0 || n < r ? mod_t(0) : fact(n) * finv(n - r); }
 // [x^r] (1 + x)^n
 static inline mod_t nCr(int n, int r) { return r < 0 || n < r ? mod_t(0) : fact(n) * finv(n - r) * finv(r); }
 // [x^r] (1 - x)^{-n}
 static inline mod_t nHr(int n, int r) { return !r ? mod_t(1) : nCr(n + r - 1, r); }
};

using namespace std;
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using Mint= ModInt<998244353>;
 using F= FactorialPrecalculation<Mint>;
 Matrix<Mint> A(8, 8);
 vector<int> in(8), out(8);
 Mint ans= 1;
 int r= 0;
 int N= 0;
 for(int s= 0; s < 16; ++s) {
  int u= s >> 1, v= s & 7, X;
  cin >> X;
  N+= X;
  A[u][v]-= X;
  A[v][v]+= X;
  out[u]+= X;
  in[v]+= X;
  ans*= F::finv(X);
  if(X) r= u;
 }
 int s= -1, t= -1;
 for(int i= 8; i--;) {
  int tmp= in[i] - out[i];
  if(abs(tmp) > 1) {
   s= -2;
  } else if(tmp == 1) {
   if(s == -1) s= i;
   else s= -2;
  } else if(tmp == -1) {
   if(t == -1) t= i;
   else s= -2;
  }
 }
 if(s == -2) return cout << 0 << '\n', 0;
 if(s != -1) {
  A[s][t]-= 1;
  A[t][t]+= 1;
  ++out[s];
  ++in[t];
 } else {
  ans*= N;
 }
 vector<int> rm= {r};
 for(int i= 8; i--;)
  if(out[i]) ans*= F::fact(out[i] - 1);
  else rm.push_back(i);
 ans*= LU_Decomposition(A.submatrix_rm(rm, rm)).det();
 cout << ans << '\n';
 return 0;
}
