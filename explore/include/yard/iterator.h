#ifndef __YARD_ITERATOR
#define __YARD_ITERATOR

namespace yard
{

template <typename Base>
class array_iterator : public Base
{
protected:
    uint8_t entry_size_;
public:
    array_iterator(const Base& base, uint8_t entry_size):
        Base(base),
        entry_size_(entry_size)
    {}

    array_iterator<Base>& operator++()
    {
        Base::pointer_ += entry_size_;
        return *this;
    }

    array_iterator<Base>& operator--()
    {
        Base::pointer_ -= entry_size_;
        return *this;
    }

    array_iterator<Base>& operator++(int)
    {
        array_iterator<Base> tmp(*this);
        operator++();
        return tmp;
    }

    array_iterator<Base>& operator--(int)
    {
        array_iterator<Base> tmp(*this);
        operator--();
        return tmp;
    }

    array_iterator<Base>& operator+=(int shift)
    {
        Base::pointer_ += shift*entry_size_;
        return *this;
    }

    array_iterator<Base>& operator-=(int shift)
    {
        Base::pointer_ -= shift*entry_size_;
        return *this;
    }

    bool operator==(const array_iterator<Base>& rhs) const
    {
        if (entry_size_ != rhs.entry_size_)
            return false;
        return (Base::pointer_ == rhs.pointer_);
    }
    
    bool operator!=(const array_iterator<Base>& rhs) const
    {
        return !(operator==(rhs));
    }

    bool operator<(const array_iterator<Base>& rhs) const
    {
        return (Base::pointer_ < (Base::rhs).pointer_);
    }

    bool operator<=(const array_iterator<Base>& rhs) const
    {
        return (Base::pointer_ <= (Base::rhs).pointer_);
    }

    bool operator>(const array_iterator<Base>& rhs) const
    {
        return (Base::pointer_ > (Base::rhs).pointer_);
    }

    bool operator>=(const array_iterator<Base>& rhs) const
    {
        return (Base::pointer_ >= (Base::rhs).pointer_);
    }
};

template <typename Base>
array_iterator<Base> operator+(const array_iterator<Base>& left, int right)
{
    array_iterator<Base> result(left);
    result += right;
    return result;
}

template <typename Base>
array_iterator<Base> operator+(int left, const array_iterator<Base>& right)
{
    return (right + left);
}

template <typename Base>
array_iterator<Base> operator-(const array_iterator<Base>& left, int right)
{
    array_iterator<Base> result(left);
    result -= right;
    return result;
}

template <typename Base>
array_iterator<Base> operator-(int left, const array_iterator<Base>& right)
{
    return (right + left);
}

}

#endif
