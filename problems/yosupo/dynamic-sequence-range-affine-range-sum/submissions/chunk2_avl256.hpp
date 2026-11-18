#pragma once
// chunk2_256.hpp の木の部分を treap から AVL 木に替えたもの (塊の大きさは 256)。中身は _chunk2avl.hpp。
#include "_chunk2avl.hpp"
using Solver = ChunkSolver2Avl<256>;
