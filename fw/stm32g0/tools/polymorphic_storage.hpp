/**
 * @file
 */

#ifndef TOOLS_POLYMORPHIC_STORAGE_HPP_
#define TOOLS_POLYMORPHIC_STORAGE_HPP_

#include <utility>
#include <type_traits>
#include <new>


template <typename T>
struct TypeTag
{
    using Type = T;
};


template <typename T, typename I, std::size_t MAX_S, std::size_t MAX_A>
concept PolymorphicStorageType =
        sizeof(T) <= MAX_S &&  // Type does not fit the storage
        alignof(T) <= MAX_A &&  // Type exceeds alignment
        std::is_base_of<I, T>::value &&  // Type needs to be an implementation of the interface
        std::has_virtual_destructor<T>::value;  // Type needs to define a virtual destructor

/**
 * @brief Object encapsulating an instance of a polymorphic object
 *
 * @tparam I Interface of the object
 * @tparam MAX_S Maximum size of an object instance
 */
template <typename I, std::size_t MAX_S, std::size_t MAX_A = alignof(void *)>
class PolymorphicStorage
{
public:
    /** @brief Interface represented by the object */
    using Interface = I;

    static inline const std::size_t MAX_SIZE = MAX_S;
    static inline const std::size_t MAX_ALIGN = MAX_A;

    PolymorphicStorage()
    {
        make<Interface>();
    }

    template <PolymorphicStorageType<Interface, MAX_SIZE, MAX_ALIGN> T, typename... Ts>
    PolymorphicStorage(TypeTag<T> tag, Ts &&... args)
    {
        make<typename decltype(tag)::Type>(std::forward<Ts>(args)...);
    }

    Interface * get() { return std::launder(reinterpret_cast<Interface *>(&storage_)); }
    const Interface * get() const { return std::launder(reinterpret_cast<const Interface *>(&storage_)); }
    Interface * operator ->() { return get(); }
    const Interface * operator ->() const { return get(); }
    Interface & operator *() { return *get(); }
    const Interface & operator *() const { return *get(); }

    template <PolymorphicStorageType<Interface, MAX_SIZE, MAX_ALIGN> T>
    void create()
    {
        get()->~Interface();
        make<T>();
    }

    template <PolymorphicStorageType<Interface, MAX_SIZE, MAX_ALIGN> T, typename... Ts>
    void create(Ts &&... args)
    {
        get()->~Interface();
        make<T>(std::forward<Ts>(args)...);
    }

    ~PolymorphicStorage()
    {
        get()->~Interface();
    }

    PolymorphicStorage(const PolymorphicStorage &) = delete;
    PolymorphicStorage(PolymorphicStorage &&) = delete;
    PolymorphicStorage & operator =(const PolymorphicStorage &) = delete;
    PolymorphicStorage & operator =(PolymorphicStorage &&) = delete;

private:
    struct StorageType
    {
        alignas (MAX_ALIGN)
        char data[MAX_SIZE];
    } storage_;

    template <PolymorphicStorageType<Interface, MAX_SIZE, MAX_ALIGN> T, typename... Ts>
    void make(Ts &&... args)
    {
        new (&storage_) T(std::forward<Ts>(args)...);
    }
};




#endif  // TOOLS_POLYMORPHIC_STORAGE_HPP_
