#pragma once

template <typename C>
class TConstArrayProxy {
public:
    typedef typename C::value_type value_type;
    typedef typename C::get_type get_type;
    typedef typename C::size_type size_type;

    TConstArrayProxy(const C* c, size_type idx)
        : c_(c), idx_(idx) {
        std::cout << "TConstArrayProxy constructor" << std::endl;
    }

    operator get_type() const {
        std::cout << "In the get_type operator." << std::endl;
        return c_->transGet(idx_);
    }

    TConstArrayProxy<C>& operator=(const TConstArrayProxy<C>&) = delete;

protected:
    const C* c_;
    size_type idx_;
};

template <typename C>
class TArrayProxy : public TConstArrayProxy<C> {
public:
    typedef TConstArrayProxy<C> base_type;
    typedef typename TConstArrayProxy<C>::value_type value_type;
    typedef typename TConstArrayProxy<C>::get_type get_type;
    typedef typename TConstArrayProxy<C>::size_type size_type;

    TArrayProxy(C* c, size_type idx)
        : base_type(c, idx) {
        std::cout << "TArrayProxy constructor" << std::endl;
    }

    TArrayProxy<C>& operator=(const value_type& x) {
        std::cout << "In the first = operator." << std::endl;
        const_cast<C*>(this->c_)->transPut(this->idx_, x);
        return *this;
    }
    TArrayProxy<C>& operator=(value_type&& x) {
        std::cout << "In the second = operator." << std::endl;
        const_cast<C*>(this->c_)->transPut(this->idx_, std::move(x));
        return *this;
    }
    TArrayProxy<C>& operator=(const TArrayProxy<C>& x) {
        std::cout << "In the thrid = operator." << std::endl;
        const_cast<C*>(this->c_)->transPut(this->idx_, x.operator get_type());
        return *this;
    }
};
