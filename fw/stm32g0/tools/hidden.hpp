/**
 * @file
 */

#ifndef TOOLS_HIDDEN_HPP_
#define TOOLS_HIDDEN_HPP_

#include <cstddef>
#include <utility>
#include <new>


template <typename T, std::size_t SZ, std::size_t AL>
concept CompleteHiddenType =
    sizeof(T) == SZ &&  // Type does not fit the storage
    alignof(T) <= AL;  // Type exceeds alignment


template <typename T, std::size_t SZ, std::size_t AL = alignof(void *)>
class Hidden
{
public:
    using Type = T;
    static const inline std::size_t ALIGN = AL;
    static const inline std::size_t SIZE = SZ;

    Type * get() { return reinterpret_cast<Type *>(&storage_); }
    const Type * get() const { return reinterpret_cast<const Type *>(&storage_); }
    Type * operator ->() { return get(); }
    const Type * operator ->() const { return get(); }
    Type & operator *() { return *get(); }
    const Type & operator *() const { return *get(); }

    template <typename... Ts>
    Hidden(Ts &&... args)
    {
        make<Type>(std::forward<Ts>(args)...);
    }

    ~Hidden()
    {
        get()->~Type();
    }

private:
    struct Storage
    {
        alignas(ALIGN)
        char data[SIZE];
    };

    Storage storage_;

    template <CompleteHiddenType<SIZE, ALIGN> AT, typename... Ts>
    void make(Ts &&... args)
    {
        new (&storage_) AT(std::forward<Ts>(args)...);
    }
};

#endif  // TOOLS_HIDDEN_HPP_
