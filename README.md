# loop_detection
Loop Detection Collections and Iterators

К хорошо известному, написанному в STL-стиле, алгоритму для определения цикла/петли в односвязном списке,

```cpp
template <class L>
bool has_cycle(L && l) {
    if (l.begin() == l.end() or std::next(l.begin()) == l.end())
        return false;
    auto slow = l.begin(); // Черепаха
    auto fast = l.begin(); // Заяц
    while (fast != l.end() and std::next(fast) != l.end()) {
        std::advance(slow, 1);
        std::advance(fast, 2);
        if (slow == fast)
            return true;
    }
    return false;
}
```
написать классы реализаций (коллекции и итераторы) в виде нескольких решений. Классы коллекций односвязных списков должны иметь конcтрукторы,
принимающие элементы через std::initializer_list<>.

Сравнить решения. Оценить достоинства и недостатки каждого.  

В обоих случаях пишется класс-коллекция looped_forward_list и класс-итератор looped_iterator.  

**Вариант 1.**

```cpp
template <class T>
struct Node {
    T value;
    shared_ptr<Node> next = nullptr;
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
```

Достоинства.  

1. Итератор имеет естественный доступ к каждому элементу узла: значению элемента и указателю на следующий элемент. То есть, уникальный адрес
(необходимый для определения петли) любого элемента всегда нам доступен.

2. Операторы сравнения, используемые в алгоритме обнаружения петли, естественным образом становятся симметричными и выражаются друг через друга.

Недостатки.  

1. Отдельный класс узла.

2. Отдельный "сырой" односвязный список.
 
3. Ввиду того, что мы принимаем через std::initializer_list<>, мы вынуждены итеративно добавлять каждый элемент в голову односвязного списка
(соблюдая канонические правила приращения односвязных списков через голову) с последующим разворотом получившегося списка с целью восстановления
исходной последовательности элементов, принятой в конструктор.

4. Перед тем как зациклить список, мы должны не забыть сохранить оригинальный следующий элемент для узла LoopFrom, и не забыть восстановить
исходный порядок узлов перед удалением коллекции. В противном случае циклическая зависимость (для случая ее реальности: c узла с большим 
порядковым номером к узлу с меньшим, но не наоборот) не даст отработать всем деструкторам корректно.


**Вариант 2.**

```cpp
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
```

Достоинства.

1. Мы пользуемся услугами низлежащего стандартного односвязного списка std::forward_list<>. В результате чего не занимаемся ничем похожим на то,
что представляло недостатки предыдущего варианта. 

Недостатки.

1. Достоинства предыдущего решения (контроль над адресом каждого элемента) теперь недоступны. Cтандартный контейнер std::forward_list нельзя
зациклить, не нарушив его контракт безопасности, поскольку контейнер управляет памятью узлов самостоятельно. Доступа к внутренним указателям
(next) для их ручного изменения нет. Это означает, что итератор лишь имитирует для нас петлю.

2. Операторы сравнения итератора не могут теперь выражаться друг через друга.

3. operator!=() условно нарушает SRP. Если это коллекция с реальной петлей (от большего порядкового номера к равному или меньшему), то в
range-based цикле конец никогда не будет достигнут, и можно смело возвращать true без дополнительных проверок. Однако, если итерация по элементам
просто перескакивает вперед (петли фактически нет), то она должна завершиться после посещения последнего элемента, и мы вынуждены нашему клиенту
вернуть false.

4. operator==(), использующийся для определения равенства быстрого и медленного итератора в алгоритме обнаружения петли, должен использовать
сырые адреса узлов std::forward_list<>. Для чего необходимо сначала разыменовать итераторы, перейдя к самим узлам, а затем взять их адреса.

5. Для реализации п.4 требуется наличие константного варианта оператора разыменования в классе-итераторе.

**Выводы.**

Несмотря на примерно одинаковое количество недостатков, второй вариант менее естественен и более сложен в кодировании, хотя и более короткий.
Можно сказать, что релизация второго варианта, несмотря на кажущийся более правильным подход к делу, в итоге нарушила принцип KISS. А сделать
проще, чем предыдущий вариант,- затруднительно.

