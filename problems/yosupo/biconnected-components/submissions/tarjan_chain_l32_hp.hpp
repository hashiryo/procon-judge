#pragma once
// tarjan_chain_hp で、同時に歩く鎖を 32 本にする。核は _bcc3.hpp。
#include "_bcc3.hpp"
using Solver= bcc3::Solver<5, 32>;
