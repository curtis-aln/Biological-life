#pragma once

#include <numeric>
#include <ranges>
#include <array>
/*
 * struct Entity;
 * struct Wrapper;
 *
 *
 */


template <class Obj>
struct Wrapper
{
    static_assert(std::is_member_object_pointer_v<decltype(&Obj::vector_id)>,
        "Obj must have a member variable named vector_id of type const unsigned");

	Obj* objPtr;
    Obj* get() { return objPtr; }

    unsigned index() { return objPtr->vector_id; }
    bool active = true;

    Wrapper(Obj* object = nullptr) : objPtr(object) {}
};


template <class Obj, std::size_t N>
class o_vector
{
    // this array contains all of the items and is never directly modified
    std::array<Wrapper<Obj>, N> array{};
    unsigned arrayRealSize = 0;
    unsigned m_indexSize = 0;

    // this vector stores all the actual objects on the heap, they are never modified or removed from. only added to
    std::vector<Obj> objectStore{};


private:
    // Iterator class definition
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Obj*;
        using difference_type = std::ptrdiff_t;
        using pointer   = Obj*;
        using reference = Obj*;

        explicit Iterator(o_vector& vec, const unsigned index) : vector(vec), currentIndex(index) {}

        Iterator& operator++()
        {
            do { ++currentIndex; } while (currentIndex < vector.array.size() && !vector.array[currentIndex].active);
            return *this;
        }

        reference operator*() const { return vector.array[currentIndex].get(); }
        pointer operator->()  const { return vector.array[currentIndex].get(); }

        bool operator==(const Iterator& other) const { return currentIndex == other.currentIndex; }
        bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        o_vector& vector;
        unsigned currentIndex = 0;
    };


    unsigned getFirstAvalableIteration()
    {
        unsigned currentIndex = 0;
        while (currentIndex < N && !array[currentIndex].active)
            ++currentIndex;
        return currentIndex;
    }

public:
    explicit o_vector() { objectStore.reserve(N); }

    Iterator begin() { return Iterator(*this, getFirstAvalableIteration()); }
    Iterator end()   { return Iterator(*this, N); }
    [[nodiscard]] unsigned size() const { return m_indexSize; }

    // used exlusevly to initilise the array, else use add()
    void emplace(Obj item)
    {
        objectStore.emplace_back(item);
	    array[arrayRealSize++] = Wrapper<Obj>(&objectStore[m_indexSize++]);
    }
    Obj* at(const unsigned i) { return array[i].get(); }


    Obj* add()
    {
        if (size() >= arrayRealSize)
            return nullptr;

        for (size_t i{ 0 }; i < arrayRealSize; ++i)
        {
            if (!array[i].active)
            {
                array[i].active = true;
                ++m_indexSize;
                return array[i].get();
            }
        }

        return nullptr;
    }

    void remove(Obj* obj) { if (obj->active) remove(obj->o_vector_index); }
    void remove(const unsigned vector_index)
    {
        array[vector_index].active = false;
        --m_indexSize;
    }
};