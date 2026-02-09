#pragma once

#include <iostream>
#include <forward_list>
#include "algo.h"

template <class T>
class looped_iterator {
    using fwl_iter = typename std::forward_list<T>::iterator;

    fwl_iter current;
    fwl_iter end;
    fwl_iter loop_to;
    fwl_iter loop_from;

public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using reference = T&;
    using const_reference = T&;  // <-------
    using pointer = T*;

    looped_iterator(fwl_iter it, fwl_iter end, fwl_iter loop_from, fwl_iter loop_to)
        : current(it), end(end), loop_from(loop_from), loop_to(loop_to) {}
    looped_iterator() = default;

    reference operator*() { return *current; }
    const_reference operator*() const { return *current; } // <-------

    looped_iterator& operator++() {
        current = current == loop_from ? loop_to : std::next(current);
        return *this;
    }

    bool operator==(looped_iterator const& other) const {
        return &**this == &*other;
    }

    bool operator!=(const looped_iterator& ) const {
        if (current == end)
            return false;
        return true;
    }
};

template <class T, std::size_t LoopFrom, std::size_t LoopTo>
class looped_forward_list {
    std::forward_list<T> base;
public:
    looped_forward_list(std::initializer_list<T> il) : base(il) {
        auto const list_size = std::distance(base.begin(), base.end());

        auto check_constraints = [list_size](std::size_t node, char const * message) {
            if (!list_size or list_size - 1 < node)
                throw std::logic_error(message);
        };

        check_constraints(LoopFrom, "Wrong 'LoopFrom' order to loop from!");
        check_constraints(LoopTo, "Wrong 'LoopTo' order to loop to!");
    }

    looped_forward_list() = delete; // список без элементов запрещаем использовать даже на этапе компиляции так как заведомо нет петли

    auto begin() {
        auto loop_from = base.begin();
        std::advance(loop_from, LoopFrom);
        auto loop_to = base.begin();
        std::advance(loop_to, LoopTo);

        return looped_iterator<T>(base.begin(), base.end(), loop_from, loop_to); 
    };
    auto end() { return looped_iterator<T>{}; };
};

