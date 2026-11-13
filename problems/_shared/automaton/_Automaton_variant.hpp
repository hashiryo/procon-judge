#pragma once
// 今の Library の Automaton (mylib/counting/Automaton.hpp、Library 594041c83) の写しで、NeoLibrary の DFA
// (neo/counting/DFA.hpp) に入れた手順を 1 つずつ替えて書き比べる (algo-notes の notes/automaton-dp.md)。include の前に
// 次のマクロを立てる。どちらも立てなければ今の Library と同じ中身になる。名前は Automaton のまま。
//   AUTOMATON_HASHMAP: 状態に番号を振る表 (コンストラクタの構築と operator& の積の表) を、64 bit に詰められる型なら
//                      NeoLibrary の HashMap、ほかは要素のハッシュを混ぜる unordered_map にする (DFA.hpp と同じ選び方の写し)
//   AUTOMATON_STEPDP:  dp_run を、状態と位置の組の待ち行列でなく、1 歩ずつ来た状態の並びを持って進める形にする
//                      (DFA.hpp の成分が 1 つのときの形の写し)。積は今の Library と同じく表にしたまま
#include <type_traits>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <queue>
#include <cstdlib>
#include <cassert>
#ifdef AUTOMATON_HASHMAP
#include <array>
#include <deque>
#include <functional>
#include <iterator>
#include <tuple>
#include <utility>
#include "neo/data_structure/HashMap.hpp"
namespace automaton_variant {
using u64= unsigned long long;
template <class T> struct is_pair: std::false_type {};
template <class A, class B> struct is_pair<std::pair<A, B>>: std::true_type {};
template <class T> struct is_tuple: std::false_type {};
template <class... A> struct is_tuple<std::tuple<A...>>: std::true_type {};
template <class T> struct is_array: std::false_type {};
template <class A, std::size_t N> struct is_array<std::array<A, N>>: std::true_type {};
template <class T> constexpr int pack_bits();
template <class T, std::size_t... I> constexpr int tuple_bits(std::index_sequence<I...>) {
 int b[]= {pack_bits<std::tuple_element_t<I, T>>()..., 0}, s= 0;
 for(int x: b) {
  if(x < 0) return -1;
  s+= x;
 }
 return s;
}
template <class T> constexpr int pack_bits() {
 if constexpr(std::is_same_v<T, bool>) return 1;
 else if constexpr(std::is_enum_v<T>) return pack_bits<std::underlying_type_t<T>>();
 else if constexpr(std::is_integral_v<T>) return sizeof(T) <= 8 ? int(sizeof(T)) * 8 : -1;
 else if constexpr(is_pair<T>::value) {
  constexpr int a= pack_bits<typename T::first_type>(), b= pack_bits<typename T::second_type>();
  return a < 0 || b < 0 ? -1 : a + b;
 } else if constexpr(is_array<T>::value) {
  constexpr int a= pack_bits<typename T::value_type>();
  return a < 0 ? -1 : a * int(std::tuple_size_v<T>);
 } else if constexpr(is_tuple<T>::value) return tuple_bits<T>(std::make_index_sequence<std::tuple_size_v<T>>());
 else return -1;
}
template <class T> constexpr bool packable= pack_bits<T>() >= 0 && pack_bits<T>() <= 64;
inline u64 shl(u64 x, int s) { return s >= 64 ? 0 : x << s; }
template <class T> u64 pack(const T& x) {
 if constexpr(std::is_same_v<T, bool>) return x;
 else if constexpr(std::is_enum_v<T>) return pack(static_cast<std::underlying_type_t<T>>(x));
 else if constexpr(std::is_integral_v<T>) return static_cast<std::make_unsigned_t<T>>(x);
 else if constexpr(is_pair<T>::value) return pack(x.first) | shl(pack(x.second), pack_bits<typename T::first_type>());
 else if constexpr(is_array<T>::value) {
  constexpr int b= pack_bits<typename T::value_type>();
  u64 r= 0;
  for(std::size_t i= 0; i < x.size(); ++i) r|= shl(pack(x[i]), b * int(i));
  return r;
 } else {
  u64 r= 0;
  int s= 0;
  std::apply([&](const auto&... a) { ((r|= shl(pack(a), s), s+= pack_bits<std::decay_t<decltype(a)>>()), ...); }, x);
  return r;
 }
}
inline u64 mix(u64 x) {
 x+= 0x9e3779b97f4a7c15ull;
 x= (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ull;
 x= (x ^ (x >> 27)) * 0x94d049bb133111ebull;
 return x ^ (x >> 31);
}
template <class T> constexpr bool hashable() {
 if constexpr(packable<T> || std::is_integral_v<T> || std::is_floating_point_v<T>) return true;
 else if constexpr(is_pair<T>::value) return hashable<typename T::first_type>() && hashable<typename T::second_type>();
 else if constexpr(is_tuple<T>::value) return []<std::size_t... I>(std::index_sequence<I...>) { return (hashable<std::tuple_element_t<I, T>>() && ...); }(std::make_index_sequence<std::tuple_size_v<T>>());
 else if constexpr(is_array<T>::value) return hashable<typename T::value_type>();
 else if constexpr(requires(const T& x) { std::hash<T>{}(x); }) return true;
 else if constexpr(requires(const T& x) { std::begin(x) != std::end(x); }) return hashable<std::decay_t<decltype(*std::begin(std::declval<const T&>()))>>();
 else return false;
}
template <class T> u64 hash_of(const T& x) {
 if constexpr(packable<T>) return mix(pack(x));
 else if constexpr(std::is_integral_v<T>) {
  const std::make_unsigned_t<T> v= x;
  u64 h= 0;
  for(std::size_t i= 0; i < sizeof(T); i+= 8) h= mix(h ^ u64(v >> (8 * i)));
  return h;
 } else if constexpr(is_pair<T>::value) return mix(hash_of(x.first) ^ mix(hash_of(x.second)));
 else if constexpr(is_tuple<T>::value || is_array<T>::value) {
  u64 h= 0;
  std::apply([&](const auto&... a) { ((h= mix(h ^ hash_of(a))), ...); }, x);
  return h;
 } else if constexpr(requires { std::hash<T>{}(x); }) return mix(std::hash<T>{}(x));
 else {
  u64 h= 0;
  for(const auto& a: x) h= mix(h ^ hash_of(a));
  return h;
 }
}
struct Hasher {
 template <class T> std::size_t operator()(const T& x) const { return hash_of(x); }
};
template <class K> struct NumPack {
 HashMap<int> h;
 int n= 0;
 std::pair<int, bool> insert(const K& k) {
  int& v= h[pack(k)];
  if(v) return {v - 1, false};
  return v= ++n, std::pair{n - 1, true};
 }
};
template <class K> struct NumHash {
 std::unordered_map<K, int, Hasher> h;
 std::pair<int, bool> insert(const K& k) {
  auto [it, ins]= h.try_emplace(k, int(h.size()));
  return {it->second, ins};
 }
};
template <class K> struct NumTree {
 std::map<K, int> h;
 std::pair<int, bool> insert(const K& k) {
  auto [it, ins]= h.try_emplace(k, int(h.size()));
  return {it->second, ins};
 }
};
template <class K> using Numbering= std::conditional_t<packable<K>, NumPack<K>, std::conditional_t<hashable<K>(), NumHash<K>, NumTree<K>>>;
}
#endif
template <class symbol_t> class Automaton {
 std::vector<int> table;
 std::vector<int8_t> info;
 std::vector<symbol_t> alph;
 const int m;
 template <class Map, class state_t, class F, class G, class H> void build(const state_t &initial_state, const F &transition, const G &is_accept, const H &abs_reject) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  static_assert(std::is_same_v<bool, std::invoke_result_t<H, state_t>>);
#ifdef AUTOMATON_HASHMAP
  automaton_variant::Numbering<state_t> encode;
  std::deque<state_t> decode;
  encode.insert(initial_state), decode.push_back(initial_state);
  for (std::size_t i= 0, k= 0; i < decode.size(); ++i) {
   const state_t &s= decode[i];
   table.resize(table.size() + m);
   for (int j= 0; j < m; ++j) {
    if (auto t= transition(s, j); abs_reject(t)) table[k++]= -1;
    else {
     auto [id, ins]= encode.insert(t);
     if (ins) decode.push_back(t);
     table[k++]= id;
    }
   }
  }
  const int ts= decode.size();
#else
  Map encode;
  std::vector<state_t> decode;
  int ts= 0;
  decode.push_back(initial_state), encode.emplace(initial_state, ts++);
  for (int i= 0, k= 0; i < ts; ++i) {
   auto s= decode[i];
   table.resize(table.size() + m);
   for (int j= 0; j < m; ++j) {
    if (auto t= transition(s, j); abs_reject(t)) table[k++]= -1;
    else if (auto it= encode.find(t); it != encode.end()) table[k++]= it->second;
    else table[k++]= ts, decode.push_back(t), encode.emplace(t, ts++);
   }
  }
#endif
  info.resize(ts);
  for (int i= ts; i--;) info[i]= is_accept(decode[i]);
 }
 Automaton(const std::vector<symbol_t> &alphabet): alph(alphabet), m(alph.size()) {}
public:
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<state_t, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept): alph(alphabet), m(alph.size()) {
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const state_t &s, int i) { return transition(s, alph[i]); };
  auto rej= [](const state_t &) { return false; };
  if constexpr (std::is_integral_v<state_t>) build<std::unordered_map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
  else build<std::map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
 }
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<state_t, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept, const state_t &abs_rej_state): alph(alphabet), m(alph.size()) {
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const state_t &s, int i) { return transition(s, alph[i]); };
  auto rej= [abs_rej_state](const state_t &s) { return s == abs_rej_state; };
  if constexpr (std::is_integral_v<state_t>) build<std::unordered_map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
  else build<std::map<state_t, int>, state_t>(initial_state, tr, is_accept, rej);
 }
 template <class state_t, class F, class G, std::enable_if_t<std::is_invocable_r_v<std::set<state_t>, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept): alph(alphabet), m(alph.size()) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  std::sort(alph.begin(), alph.end());
  auto tr= [&](const std::set<state_t> &s, int i) {
   std::set<state_t> ret;
   for (const auto &x: s) {
    auto ys= transition(x, alph[i]);
    ret.insert(ys.begin(), ys.end());
   }
   return ret;
  };
  auto ac= [&](const std::set<state_t> &s) { return std::any_of(s.begin(), s.end(), is_accept); };
  auto rej= [](const std::set<state_t> &s) { return s == std::set<state_t>(); };
  build<std::map<std::set<state_t>, int>, std::set<state_t>>(std::set<state_t>({initial_state}), tr, ac, rej);
 }
 template <class state_t, class F, class G, class H, std::enable_if_t<std::is_invocable_r_v<std::set<state_t>, F, state_t, symbol_t>, std::nullptr_t> = nullptr> Automaton(const std::vector<symbol_t> &alphabet, const state_t &initial_state, const F &transition, const G &is_accept, const H &eps_trans): alph(alphabet), m(alph.size()) {
  static_assert(std::is_same_v<bool, std::invoke_result_t<G, state_t>>);
  static_assert(std::is_same_v<std::set<state_t>, std::invoke_result_t<H, state_t>>);
  std::sort(alph.begin(), alph.end());
  auto eps_closure= [&](std::set<state_t> s) {
   for (std::set<state_t> t; s != t;) {
    t= s;
    for (const auto &x: t) {
     auto ys= eps_trans(x);
     s.insert(ys.begin(), ys.end());
    }
   }
   return s;
  };
  auto tr= [&](const std::set<state_t> &s, int i) {
   std::set<state_t> ret;
   for (const auto &x: s) {
    auto ys= transition(x, alph[i]);
    ret.insert(ys.begin(), ys.end());
   }
   return eps_closure(ret);
  };
  auto ac= [&](const std::set<state_t> &s) { return std::any_of(s.begin(), s.end(), is_accept); };
  auto rej= [](const std::set<state_t> &s) { return s == std::set<state_t>(); };
  build<std::map<std::set<state_t>, int>, std::set<state_t>>(eps_closure({initial_state}), tr, ac, rej);
 }
 Automaton operator&(const Automaton &r) const {
  assert(alph == r.alph);
  const int S= info.size();
  auto tr= [&](int s, int q) {
   auto [s1, s0]= std::div(s, S);
   int t1= r.table[s1 * m + q], t0= table[s0 * m + q];
   return t0 == -1 || t1 == -1 ? -1 : t1 * S + t0;
  };
  auto ac= [&](int s) {
   auto [s1, s0]= std::div(s, S);
   return info[s0] == 1 && r.info[s1] == 1;
  };
  auto rej= [](int s) { return s == -1; };
  Automaton ret(alph);
  return ret.build<std::unordered_map<int, int>, int>(0, tr, ac, rej), ret;
 }
 template <class T, class A, class F> T dp_run(int n, const A &op, const T &ti, const F &f, const T &init) const {
  static_assert(std::is_same_v<T, std::invoke_result_t<A, T, T>>);
  static_assert(std::is_same_v<T, std::invoke_result_t<F, T, symbol_t, int>>);
#ifdef AUTOMATON_STEPDP
  std::vector<int> slot(info.size()), key{0}, nkey;
  std::vector<T> val{init}, nval;
  for (int i= 0; i < n && key.size(); ++i) {
   nkey.clear(), nval.clear();
   for (std::size_t s= 0; s < key.size(); ++s) {
    const int *row= table.data() + std::size_t(key[s]) * m;
    const T &v= val[s];
    for (int c= 0; c < m; ++c) {
     const int t= row[c];
     if (t < 0) continue;
     if (int &p= slot[t]; !p) p= int(nkey.size()) + 1, nkey.push_back(t), nval.push_back(f(v, alph[c], i));
     else nval[p - 1]= op(nval[p - 1], f(v, alph[c], i));
    }
   }
   for (int t: nkey) slot[t]= 0;
   key.swap(nkey), val.swap(nval);
  }
  T ret= ti;
  for (std::size_t s= 0; s < key.size(); ++s)
   if (info[key[s]] == 1) ret= op(ret, val[s]);
  return ret;
#else
  const size_t S= info.size();
  std::queue<std::pair<int, int>> que;
  T dp[2][S], ret= ti;
  bool in[2][S];
  for (std::fill_n(dp[0], S, ti), std::fill_n(dp[1], S, ti), std::fill_n(in[0], S, 0), std::fill_n(in[1], S, 0), dp[0][0]= init, que.emplace(0, 0), in[0][0]= 1; que.size();) {
   auto [s, i]= que.front();
   bool b= i & 1;
   T tmp= dp[b][s];
   if (que.pop(), in[b][s]= 0, dp[b][s]= ti; i == n) {
    if (info[s] == 1) ret= op(ret, tmp);
    continue;
   }
   auto l= table.cbegin() + s * m;
   for (int j= m; j--;)
    if (int t= l[j]; t != -1)
     if (dp[!b][t]= op(dp[!b][t], f(tmp, alph[j], i)); !in[!b][t]) que.emplace(t, i + 1), in[!b][t]= 1;
  }
  return ret;
#endif
 }
 template <class T> T num(int n) const {
  return dp_run(n, [](const T &l, const T &r) { return l + r; }, T(), [](const T &x, const auto &, auto) { return x; }, T(1));
 }
};
