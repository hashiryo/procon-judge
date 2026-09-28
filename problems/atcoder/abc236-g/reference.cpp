// 期待出力を作る参照実装。submissions/lib.cpp を pj bundle で 1 ファイルに展開して固定したもの
// (2026-09-27、Library ee5e64302)。Library を直しても変わらないので、直したあとの提出はこれと比べられる。
// 小さい入力では brute.cpp と突き合わせてある (pj testdata crosscheck)。

#if defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <bits/allocator.h>
#pragma GCC target("sse3,ssse3,sse4.1,sse4.2,popcnt,cx16,sahf,avx,avx2,bmi,bmi2,fma,f16c,lzcnt,movbe,pclmul,vpclmulqdq")
#endif

// (min,max)半環
#include <iostream>
#include <iostream>
#include <type_traits>
#define _DETECT_BOOL(name, ...) \
 template <class, class= void> struct name: std::false_type {}; \
 template <class T> struct name<T, std::void_t<__VA_ARGS__>>: std::true_type {}; \
 template <class T> static constexpr bool name##_v= name<T>::value
#define _DETECT_TYPE(name, type1, type2, ...) \
 template <class T, class= void> struct name { \
  using type= type2; \
 }; \
 template <class T> struct name<T, std::void_t<__VA_ARGS__>> { \
  using type= type1; \
 }

template <class M> struct Algebra {
 using T= typename M::T;
 _DETECT_BOOL(has_zero, decltype(T::o));
 _DETECT_BOOL(has_one, decltype(T::i));
 static inline T zero= has_zero_v<M> ? M::o : T();
 static inline T one= has_one_v<M> ? M::i : T();
 T x;
 Algebra(): x(zero) {}
 Algebra(bool y): x(y ? one : zero) {}
 template <class U, typename= std::enable_if_t<std::is_convertible_v<U, T>>> Algebra(U y): x(y) {}
 Algebra& operator+=(const Algebra& r) { return *this= *this + r; }
 Algebra& operator-=(const Algebra& r) { return *this= *this - r; }
 Algebra& operator*=(const Algebra& r) { return *this= *this * r; }
 Algebra operator+(const Algebra& r) const { return Algebra(M::add(x, r.x)); }
 Algebra operator-(const Algebra& r) const { return Algebra(M::add(x, M::neg(r.x))); }
 Algebra operator*(const Algebra& r) const { return Algebra(M::mul(x, r.x)); }
 Algebra operator-() const { return Algebra(M::neg(x)); }
 bool operator==(const Algebra& r) const { return x == r.x; }
 bool operator!=(const Algebra& r) const { return x != r.x; }
 friend std::istream& operator>>(std::istream& is, Algebra& r) { return is >> r.x, is; }
 friend std::ostream& operator<<(std::ostream& os, const Algebra& r) { return os << r.x; }
};

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

using namespace std;
struct M {
 using T= int;
 static constexpr T o= 1 << 30, i= 0;
 static T add(T a, T b) { return min(a, b); }
 static T mul(T a, T b) { return max(a, b); }
};
signed main() {
 cin.tie(0);
 ios::sync_with_stdio(0);
 using R= Algebra<M>;
 int N, T, L;
 cin >> N >> T >> L;
 Matrix<R> A(N, N);
 for(int t= 1; t <= T; t++) {
  int u, v;
  cin >> u >> v, u--, v--;
  A[v][u]= t;
 }
 A= A.pow(L);
 for(int i= 0; i < N; i++) cout << (A[i][0] == R(false) ? -1 : A[i][0]) << " \n"[i == N - 1];
 return 0;
}
