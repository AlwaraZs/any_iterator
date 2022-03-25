#include <cstring>
#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

template <typename T, typename Tag>
struct any_iterator {
    // replace void with needed type
    using value_type = T;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = int;
    using iterator_category = Tag;

    any_iterator() = default;

    template <typename It,
            typename = std::enable_if_t<std::is_same_v<
                    Tag, typename std::iterator_traits<It>::iterator_category>>>
    any_iterator(It it) : _iterator(new ItErase<It>{it}) {
        _manager.clone = &clone<It>;
    }

    any_iterator(any_iterator const& other)
            : _manager(other._manager), _iterator(other.manager().clone(*other._iterator, _stack)) {
    }

    any_iterator(any_iterator&& other) : _iterator(other._iterator) {
        std::memcpy(&_manager, &other._manager,
                    sizeof(_manager));
        other._iterator = nullptr;
    }

    any_iterator& operator=(any_iterator&& other) {
        swap(other);
        return *this;
    }

    any_iterator& operator=(any_iterator const& other) {
        any_iterator temp(other);
        swap(temp);
        return *this;
    }

    void swap(any_iterator& other) noexcept {
        SwapImpl(other);
    }

    template <typename It>
    std::enable_if_t<
            std::is_same_v<Tag, typename std::iterator_traits<It>::iterator_category>,
            any_iterator&>
    operator=(It it) {
        _iterator = new ItErase<It>{it};
        _manager.clone = &clone<It>;
        return *this;
    }

    T const& operator*() const {
        return **_iterator;
    }

    T& operator*() {
        return **_iterator;
    }

    T const* operator->() const {
        return (*_iterator).operator->();
    }

    T* operator->() {
        return (*_iterator).operator->();
    }

    any_iterator& operator++() & {
        ++*_iterator;
        return *this;
    }

    any_iterator operator++(int) & {
        any_iterator ret(*this);
        ++*this;
        return ret;
    }

    template <typename TT, typename TTag>
    friend bool operator==(any_iterator<TT, TTag> const& a,
                           any_iterator<TT, TTag> const& b);

    template <typename TT, typename TTag>
    friend bool operator!=(any_iterator<TT, TTag> const& a,
                           any_iterator<TT, TTag> const& b);

    // note: next operators must compile ONLY for appropriate iterator tags

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::bidirectional_iterator_tag, TTag>,
            any_iterator<TT, TTag>&>
    operator--(any_iterator<TT, TTag>&);

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::bidirectional_iterator_tag, TTag>,
            any_iterator<TT, TTag>>
    operator--(any_iterator<TT, TTag>&, int);

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::random_access_iterator_tag, TTag>,
            any_iterator<TT, TTag>&>
    operator+=(any_iterator<TT, TTag>&,
               typename any_iterator<TT, TTag>::difference_type);

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::random_access_iterator_tag, TTag>,
            any_iterator<TT, TTag>&>
    operator-=(any_iterator<TT, TTag>&,
               typename any_iterator<TT, TTag>::difference_type);

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::random_access_iterator_tag, TTag>,
            any_iterator<TT, TTag>>
    operator+(any_iterator<TT, TTag>,
              typename any_iterator<TT, TTag>::difference_type);

    template <typename TT, typename TTag>
    friend std::enable_if_t<
            std::is_base_of_v<std::random_access_iterator_tag, TTag>,
            any_iterator<TT, TTag>>
    operator-(any_iterator<TT, TTag>,
              typename any_iterator<TT, TTag>::difference_type);

    template <typename TT, typename TTag>
    friend bool operator<(any_iterator<TT, TTag> const& a,
                          any_iterator<TT, TTag> const& b);

    template <typename TT, typename TTag>
    friend typename any_iterator<TT, TTag>::difference_type
    operator-(any_iterator<TT, TTag> const& a, any_iterator<TT, TTag> const& b);

    ~any_iterator() {
        if (!small()) {
            delete _iterator;
        } else {
            _iterator->~EraseItBase();
        }
    }

private:
    struct EraseItBase {
        virtual EraseItBase& operator++() = 0;

        virtual EraseItBase& operator--() = 0;

        virtual reference operator*() = 0;

        virtual const value_type& operator*() const = 0;

        virtual EraseItBase& operator+=(difference_type n) = 0;

        virtual EraseItBase& operator-=(difference_type n) = 0;

        virtual bool operator==(EraseItBase&) = 0;

        virtual pointer operator->() = 0;

        virtual const value_type* operator->() const = 0;

        virtual bool operator<(EraseItBase&) = 0;

        virtual difference_type operator-(EraseItBase&) = 0;

        virtual ~EraseItBase() = default;
    };

    template <class It>
    struct ItErase : EraseItBase {
        explicit ItErase(It it) : _it(it) {}

        ItErase(ItErase const&) = default;

        ItErase& operator++() override {
            if constexpr (std::is_base_of_v<
                    std::forward_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                ++_it;
            }
            return *this;
        }

        ItErase& operator--() override {
            if constexpr (std::is_base_of_v<
                    std::bidirectional_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                --_it;
            }
            return *this;
        }

        T& operator*() override {
            return *_it;
        }

        const T& operator*() const override {
            return *_it;
        }

        EraseItBase& operator+=(difference_type n) override {
            if constexpr (std::is_base_of_v<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                _it += n;
            }
            return *this;
        }

        EraseItBase& operator-=(difference_type n) override {
            if constexpr (std::is_base_of_v<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                _it -= n;
            }
            return *this;
        }

        bool operator<(EraseItBase& other) override {
            if constexpr (std::is_base_of_v<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                return _it < static_cast<ItErase&>(other)._it;
            }
            return false;
        }

        difference_type operator-(EraseItBase& other) override {
            if constexpr (std::is_base_of_v<
                    std::random_access_iterator_tag,
                    typename std::iterator_traits<It>::iterator_category>) {
                return _it - static_cast<ItErase&>(other)._it;
            }
            return false;
        }

        bool operator==(EraseItBase& other) override {
            return _it == static_cast<ItErase&>(other)._it;
        }

        T* operator->() override {
            return _it.operator->();
        }

        const T* operator->() const override {
            return _it.operator->();
        }

        It _it;
    };

    using StackStorage = std::aligned_storage_t<sizeof(ItErase<void*>)>;

    void SwapImpl(any_iterator& other) noexcept {
        std::swap(_manager, other._manager);
        std::swap(other._iterator, _iterator);
    }

    template <class It>
    EraseItBase* construct(It it) {
        if constexpr (sizeof(ItErase<It>) <= sizeof(StackStorage)) {
            ::new (&_stack) ItErase<It>(it);
            return (EraseItBase*)&_stack;
        }
        return new ItErase<It>(it);
    }

    struct Manager {
        EraseItBase* (*clone)(const EraseItBase&, StackStorage& st);
    };

    template<class It>
    static EraseItBase* clone(const EraseItBase& it, StackStorage& st) {
        if constexpr (sizeof(It) < sizeof(StackStorage)) {
            ::new (&st) ItErase<It>(static_cast<const ItErase<It>&>(it));
            return (EraseItBase*)&st;
        }
        return new ItErase<It>(static_cast<const ItErase<It>&>(it));
    }

    Manager manager() const {
        return _manager;
    }

    bool small() {
        return _iterator == (EraseItBase*)&_stack;
    }

    Manager _manager{};
    StackStorage _stack{};
    EraseItBase* _iterator{};
};

template <typename TT, typename TTag>
bool operator==(const any_iterator<TT, TTag>& a,
                const any_iterator<TT, TTag>& b) {
    return *a._iterator == *b._iterator;
}

template <typename TT, typename TTag>
bool operator!=(const any_iterator<TT, TTag>& a,
                const any_iterator<TT, TTag>& b) {
    return !(a == b);
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, TTag>,
        any_iterator<TT, TTag>&>
operator--(any_iterator<TT, TTag>& it) {
    --*it._iterator;
    return it;
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::bidirectional_iterator_tag, TTag>,
        any_iterator<TT, TTag>>
operator--(any_iterator<TT, TTag>& it, int) {
    any_iterator<TT, TTag> res(it);
    --it;
    return res;
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, TTag>,
        any_iterator<TT, TTag>&>
operator+=(any_iterator<TT, TTag>& it,
           typename any_iterator<TT, TTag>::difference_type n) {
    *it._iterator += n;
    return it;
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, TTag>,
        any_iterator<TT, TTag>&>
operator-=(any_iterator<TT, TTag>& it,
           typename any_iterator<TT, TTag>::difference_type n) {
    *it._iterator -= n;
    return it;
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, TTag>,
        any_iterator<TT, TTag>>
operator+(any_iterator<TT, TTag> it,
          typename any_iterator<TT, TTag>::difference_type n) {
    auto temp = it;
    *temp._iterator += n;
    return temp;
}

template <typename TT, typename TTag>
std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, TTag>,
        any_iterator<TT, TTag>>
operator-(any_iterator<TT, TTag> it,
          typename any_iterator<TT, TTag>::difference_type n) {
    *it._iterator -= n;
    return it;
}

template <typename TT, typename TTag>
bool operator<(const any_iterator<TT, TTag>& a,
               const any_iterator<TT, TTag>& b) {
    return *a._iterator < *b._iterator;
}

template <typename TT, typename TTag>
bool operator<=(const any_iterator<TT, TTag>& a,
                const any_iterator<TT, TTag>& b) {
    return a < b || a == b;
}

template <typename TT, typename TTag>
bool operator>(const any_iterator<TT, TTag>& a,
               const any_iterator<TT, TTag>& b) {
    return !(a <= b);
}

template <typename TT, typename TTag>
bool operator>=(const any_iterator<TT, TTag>& a,
                const any_iterator<TT, TTag>& b) {
    return !(a < b);
}

template <typename TT, typename TTag>
typename any_iterator<TT, TTag>::difference_type
operator-(const any_iterator<TT, TTag>& a, const any_iterator<TT, TTag>& b) {
    return *a._iterator - *b._iterator;
}