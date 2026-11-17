// 診断用。lib-neo-hp と同じ mmap の道で、2 MB 以上の確保を 4 KB のページに留める (_shared/pages/plain_new.hpp)。lib-neo-hp
// との差がページの大きさの分になる。比べ終えたら消す。
#define PAGES_4K
#include "_shared/pages/plain_new.hpp"
#include "lib-neo.cpp"
