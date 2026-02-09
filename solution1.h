#pragma once

#include <iostream>
#include <forward_list>
#include <memory>
#include <utility>
#include "algo.h"

using std::shared_ptr;

template <class T>
struct Node {
    T value;
    shared_ptr<Node> next = nullptr;
    ~Node() { /*std::cout << "dest\n";*/ }
};

template <class T>
class looped_iterator {
    shared_ptr<Node<T>> node = nullptr;
public:
    // value_type, difference_type, iterator_category нужны для std::distance()
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;
    using reference = T&;
    using pointer = T*;

    looped_iterator(shared_ptr<Node<T>> node) : node(std::move(node)) {}
    looped_iterator(Node<T>* node) : node(node) {}

    reference operator*() {
        return node->value;
    }

    looped_iterator& operator++() {
        node = node->next;
        return *this;
    }

    bool operator==(looped_iterator const& other) const {
        return !operator!=(other);
    }

    bool operator!=(looped_iterator const& other) const {
        return node != other.node; // OK = будет сравнивать node.get() и  other.node.get()
    }
};

template <class T, std::size_t LoopFrom, std::size_t LoopTo>
class looped_forward_list {
    shared_ptr<Node<T>> head;
public:
    looped_forward_list(std::initializer_list<T> il) : head(nullptr) {
        for (auto && e : il)
            push_front(e);
        reverse();

        auto const list_size = std::distance(begin(), end());

        auto check_constraints = [list_size](std::size_t node, char const * message) {
            if (!list_size or list_size - 1 < node)
                throw std::logic_error(message);
        };

        check_constraints(LoopFrom, "Wrong 'LoopFrom' order to loop from!");
        check_constraints(LoopTo, "Wrong 'LoopTo' order to loop to!");

        save_original_next();
        get_n_node(LoopFrom)->next = get_n_node(LoopTo);
    }

    looped_forward_list() = delete; // список без элементов запрещаем использовать даже на этапе компиляции так как заведомо нет петли

    ~looped_forward_list() {
        restore_original_next();
    }

private:
    void push_front(T const &data) {
        head.reset(new Node<T>{data, head});
    }
    void reverse() {
        shared_ptr<Node<T>> prev = nullptr;
        auto current = head;
        shared_ptr<Node<T>> next;
        while(current != nullptr) {
            next = current->next;
            current->next = prev;
            prev = current;
            current = next;
        }
        head = prev;
    }
    [[nodiscard]] shared_ptr<Node<T>> get_n_node(std::size_t n) const {
        auto node = head;
        while(n--)
           node = node->next;
        return node;
    }
    shared_ptr<Node<T>> original_next;
    void save_original_next() {
        original_next = get_n_node(LoopFrom)->next;
    }
    void restore_original_next() const {
        get_n_node(LoopFrom)->next = original_next;
    }

public:
    [[nodiscard]] looped_iterator<T> begin() {return head; }
    [[nodiscard]] looped_iterator<T> end() { return nullptr; }
};

