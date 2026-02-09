#pragma once

template <class L>
bool has_cycle(L && l) {
    if (l.begin() == l.end() or std::next(l.begin()) == l.end()) // используется operator==
        return false;
    auto slow = l.begin(); // Черепаха
    auto fast = l.begin(); // Заяц
    while (fast != l.end() and std::next(fast) != l.end()) {
        std::advance(slow, 1);
        std::advance(fast, 2);
        if (slow == fast)  // используется operator==
            return true;
    }
    return false;
}
