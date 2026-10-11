#pragma once
// tsin と同じ Tsin の吸収の形の DFS を、2 MB 境界の 1 本の作業領域 (huge page を頼む) の u32 の配列と CSR で回す。核は _t3.hpp。
#include "_t3.hpp"
using Solver= t3::Solver<false>;
