#if !defined(__ARRAY_LIST_H__)
#define __ARRAY_LIST_H__

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <fstream>
#include <cstring>
#include "user_assert.h"

namespace util
{

using std::string;
using std::ifstream;
using std::ofstream;
using std::list;
using std::vector;
using std::set;
using std::pair;

// If update is executed, the follow conditions must be fit.
// T must have public member variable : static const int field_offset[...],
// and : static const data_type field_type[...].
// T may have public member variable : static const char* select_name[...] if sql statement is provided.
template <typename T, size_t N = 10000>
class array_list  
{
public:
    class item_iter;

    class block_t
    {
    public:
        typedef T value_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;

        block_t()
        {
            _M_start = new T[N];
            _M_finish = _M_start;
            _M_end_of_storage = _M_start + N;
        }
        
        block_t(const block_t& __value)
        {
            _M_start = new T[N];
            _M_finish = _M_start + __value.size();
            _M_end_of_storage = _M_start + N;
            if (_M_start != _M_finish)
                memcpy(_M_start, __value._M_start, __value.size() * sizeof(T));
        }
        
        ~block_t()
        {
            delete[] _M_start;
        }

        block_t& operator=(const block_t& __value)
        {
            _M_finish = _M_start + __value.size();
            if (_M_start != _M_finish)
                memcpy(_M_start, __value._M_start, __value.size() * sizeof(T));
        }

        iterator begin() { return _M_start; }
        const_iterator begin() const { return _M_start; }
        iterator end() { return _M_finish; }
        const_iterator end() const { return _M_finish; }
        size_t size() const { return _M_finish - _M_start; }
        bool empty() const { return begin() == end(); }
        bool full() const { return _M_finish == _M_end_of_storage; }
        reference operator[](size_type __n) { return *(begin() + __n); }
        const_reference operator[](size_type __n) const { return *(begin() + __n); }
        
        void assign(size_type __n, const T& __val)
        {
            assert(_M_start + __n < _M_end_of_storage);
            _M_start[__n] == __val;
        }
        
        void push_back(const T& __x)
        {
            assert(_M_finish != _M_end_of_storage);
            *_M_finish = __x;
            ++_M_finish;
        }

        iterator insert(iterator __position, const T& __x)
        {
            assert(__position < _M_end_of_storage);
            *__position = __x;
            if (__position == end())
                ++_M_finish;
            return __position;
        }

        iterator insert(iterator __position, size_type __n, const T& __x)
        {
            assert(__position + __n <= _M_end_of_storage);
            for (iterator iter = __position; iter < __position + __n; ++iter)
                *iter = __x;
            if (__position + __n > end())
                _M_finish = __position + __n -1;
            return __position;
        }

        void pop_back()
        {
            --_M_finish;
        }
        
        iterator erase(iterator __position)
        {
            if (__position + 1 != end())
                std::copy(__position + 1, _M_finish, __position);
            --_M_finish;
            return __position;
        }
        
        iterator erase(iterator __first, iterator __last)
        {
            iterator __i = std::copy(__last, _M_finish, __first);
            _M_finish = _M_finish - (__last - __first);
            return __first;
        }

        void resize(size_type __new_size, const T& __x)
        {
            if (__new_size < size())
                erase(begin() + __new_size, end());
            else
                insert(end(), __new_size - size(), __x);
        }
        
        void resize(size_type __new_size) { resize(__new_size, T()); }
        void clear() { erase(begin(), end()); }
  
    private:
        T* _M_start;
        T* _M_finish;
        T* _M_end_of_storage;
    };

    typedef block_t value_type;
    typedef typename std::list<value_type>::size_type size_type;
    typedef typename std::list<value_type>::difference_type difference_type;
    typedef typename std::list<value_type>::reference reference;
    typedef typename std::list<value_type>::const_reference const_reference;
    typedef typename std::list<value_type>::pointer pointer;
    //typedef typename std::list<value_type>::const_pointer const_pointer;
    typedef typename std::list<value_type>::const_pointer const_pointer;
    typedef typename std::list<value_type>::iterator iterator;
    typedef typename std::list<value_type>::const_iterator const_iterator;
    typedef typename std::list<value_type>::reverse_iterator reverse_iterator;
    typedef typename std::list<value_type>::const_reverse_iterator const_reverse_iterator;
    typedef item_iter item_iterator;
    typedef const item_iter const_item_iterator;

    // Using this constructor can't call function update, and no need to provide any T's static member.
    array_list() {}
    virtual ~array_list() {}

    iterator begin() { return __list.begin(); }
    iterator end() { return  __list.end(); }
    const_iterator begin() const { return __list.begin();}
    const_iterator end() const { return  __list.end(); }
    size_type size() const { return __list.size(); }
    
    item_iterator item_begin()
    {
        item_iter iter;
        iter.node.first = __list.begin();
        if (iter.node.first != __list.end())
            iter.node.second = iter.node.first->begin();
        else
            iter.node.second = 0;
        iter._M_end_of_storage = __list.end();
        return iter;
    }

    const_item_iterator item_begin() const
    {
        item_iter iter;
        iter.node.first = __list.begin();
        if (iter.node.first != __list.end())
            iter.node.second = iter.node.first->begin();
        else
            iter.node.second = 0;
        iter._M_end_of_storage = __list.end();
        return iter;
    }

    item_iterator item_end()
    {
        item_iter iter;
        iter.node.first = __list.end();
        iter.node.second = 0;
        iter._M_end_of_storage = __list.end();
        return iter;
    }

    const_item_iterator item_end() const
    {
        item_iter iter;
        iter.node.first = __list.end();
        iter.node.second = 0;
        iter._M_end_of_storage = __list.end();
        return iter;
    }
    
    void push_back(const T& __x)
    {
        reverse_iterator riter = __list.rbegin();
        if (riter == __list.rend() || riter->full())
        {
            iterator iter;
            iter = __list.insert(__list.end(), block_t());
            iter->push_back(__x);
        }
        else
        {
            riter->push_back(__x);
        }
    }

    size_type item_size() const
    {
        size_type count = 0;
        for (const_iterator iter = __list.begin(); iter != __list.end(); ++iter)
            count += iter->size();
        return count;
    }

    void insert(iterator __position, const block_t* __first, const block_t* __last) { __list.insert(__position, __first, __last); }
    void insert(iterator __position, const_iterator __first, const_iterator __last) { __list.insert(__position, __first, __last); }
    void insert(iterator __position, size_type __n, const block_t& __x) { __list.insert(__position, __n, __x); }
    typename block_t::iterator insert(const T& __x)
    {
        push_back(__x);
        return (__list.rbegin()->end() - 1);
    }

    void push_back(const block_t& __x) { __list.push_back(__x); }
    void push_front(const block_t& __x) { __list.push_front(__x); }
    void pop_back() { __list.pop_back(); }
    void pop_front() { __list.pop_front(); }
    iterator erase(iterator __position) { return __list.erase(__position); }
    iterator erase(iterator __first, iterator __last) { return __list.erase(__first, __last); }
    void resize(size_type __new_size) { resize(__new_size, block_t()); }
    void clear() { erase(begin(), end()); }

    // class FUN must have operator(T&) override.
    template <typename FUN>
    void for_each(const FUN& fun)
    {
        iterator outer_iter;
        for (outer_iter = __list.begin(); outer_iter != __list.end(); ++outer_iter)
        {
            typename block_t::iterator inner_iter;
            for (inner_iter = outer_iter->begin(); inner_iter != outer_iter->end(); ++inner_iter)
                fun(*inner_iter);
        }
    }

    void for_each(void (*fun)(T&))
    {
        iterator outer_iter;
        for (outer_iter = __list.begin(); outer_iter != __list.end(); ++outer_iter)
        {
            typename block_t::iterator inner_iter;
            for (inner_iter = outer_iter->begin(); inner_iter != outer_iter->end(); ++inner_iter)
                fun(*inner_iter);
        }
    }

    bool dump(const string& file_name)
    {
        ofstream ofs(file_name.c_str());
        if (!ofs)
            return false;
        iterator iter;
        for (iter = __list.begin(); iter != __list.end(); ++iter)
        {
            int block_size = iter->end() - iter->begin();
            ofs.write(reinterpret_cast<char*>(iter->begin()), block_size * sizeof(T));
        }
        if (!ofs)
            return false;
        else
            return true;
    }

    bool load(const string& file_name)
    {
        ifstream ifs(file_name.c_str());
        if (!ifs)
            return false;
        T tmp;
        while (1)
        {
            ifs.read(reinterpret_cast<char*>(&tmp), sizeof(T));
            if (!ifs)
                break;
            push_back(tmp);
        }
        return true;
    }

    class item_iter
    {
    public:
        typedef T value_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef typename array_list<T, N>::iterator outer_iter;
        typedef typename array_list<T, N>::block_t::iterator inner_iter;
        typedef pair<outer_iter, inner_iter>* iterator;
        typedef const pair<outer_iter, inner_iter>* const_iterator;

        item_iter() {}
        ~item_iter() {}

        item_iter& operator++()
        {
            ++node.second;
            if (node.second == node.first->end())
            {
                ++node.first;
                if (node.first != _M_end_of_storage)
                    node.second = node.first->begin();
                else
                    node.second = 0;
            }
            return *this;
        }

        const item_iter operator++(int)
        {
            item_iter tmp = *this;
            ++node.second;
            if (node.second == node.first->end())
            {
                ++node.first;
                if (node.first != _M_end_of_storage)
                    node.second = node.first->begin();
                else
                    node.second = 0;
            }
            return tmp;
        }

        reference operator*()
        {
            return *node.second;
        }

        const_reference operator*() const
        {
            return *node.second;
        }

        pointer operator->()
        {
            return node.second;
        }

        const_pointer operator->() const
        {
            return node.second;
        }

        difference_type operator-(const item_iter&) const
        {
            return 0;
        }

        bool operator==(const item_iter& rhs) const
        {
            if (node.first != rhs.node.first)
                return false;
            else if (node.second != rhs.node.second)
                return false;
            else
                return true;
        }

        bool operator!=(const item_iter& rhs) const
        {
            return !(*this == rhs);
        }

        std::pair<outer_iter, inner_iter> node;
        outer_iter _M_end_of_storage;
    };

private:
    list<value_type> __list;
};

}

#endif

