// 診断用。lib-neo と同じ機械語の本体に、2 MB 以上の確保を 2 MB 境界の mmap に置いて huge page を頼む operator new を足す
// (_shared/pages/plain_new.hpp)。DFA の表 (200 万状態 × 19 記号の int、152 MB) を 4 KB のページで書く分を、同じジョブで
// 測る lib-neo-4k と比べる。比べ終えたら消す。
#define PAGES_HP
#include "_shared/pages/plain_new.hpp"
#include "lib-neo.cpp"
