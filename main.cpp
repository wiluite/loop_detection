#if 1
#include "solution1.h"
#else
#include "solution2.h"
#endif
#include <cassert>

int main() {
    try {
        int break_cycle_after = 200;
        for (auto e : looped_forward_list<int, 6, 2> {12, 14, 16, 18, 20, 22, 24, 26}) {
            std::cout << e << ' ';
            if (!--break_cycle_after)
                break;
        }
        std::cout << std::endl;
        assert (has_cycle(looped_forward_list<int, 6, 2>{12, 14, 16, 18, 20, 22, 24, 26}));
        assert (!has_cycle(looped_forward_list<int, 2, 6>{12, 14, 16, 18, 20, 22, 24, 26}));
        assert (!has_cycle(std::forward_list<int>{12, 14, 16, 18, 20, 22, 24, 26}));
    } catch(std::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}

