/**
 * @file
 */

#ifndef TOOLS_SERDES_HPP_
#define TOOLS_SERDES_HPP_

#include <cstddef>
#include <cstring>

#include <type_traits>


/**
 * @brief Tool used to serialize data
 */
class Serializer
{
public:
    explicit Serializer(void * buffer = nullptr, std::size_t capacity = 0):
        pos_(reinterpret_cast<char *>(buffer)), end_(reinterpret_cast<char *>(buffer) + capacity)
    { }
    Serializer(const Serializer &) = default;
    Serializer & operator =(const Serializer &) = default;

    std::size_t processed(void * buffer) const
    {
        return nullptr != pos_ ? pos_ - reinterpret_cast<char *>(buffer) : 0;
    }

    std::size_t remaining() const
    {
        return end_ - pos_;
    }

    bool serialize(const void * buffer, std::size_t length)
    {
        if (length > remaining())
        {
            // Mark de-serialization as failed
            pos_ = nullptr;
            end_ = nullptr;
            return false;
        }
        std::memcpy(pos_, buffer, length);
        pos_ += length;
        return true;
    }

    template <typename T>
    bool serialize(const T * value)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Value needs to be trivially copyable");
        return serialize(value, sizeof(T));
    }

private:
    char * pos_;
    char * end_;
};


/**
 * @brief Tool used to de-serialize data
 */
class Deserializer
{
public:
    explicit Deserializer(const void * buffer = nullptr, std::size_t capacity = 0):
        pos_(reinterpret_cast<const char *>(buffer)), end_(reinterpret_cast<const char *>(buffer) + capacity)
    { }
    Deserializer(const Deserializer &) = default;
    Deserializer & operator =(const Deserializer &) = default;

    std::size_t processed(const void * buffer) const
    {
        return nullptr != pos_ ? pos_ - reinterpret_cast<const char *>(buffer) : 0;
    }

    std::size_t remaining() const
    {
        return end_ - pos_;
    }

    bool deserialize(void * buffer, std::size_t length)
    {
        if (length > remaining())
        {
            // Mark de-serialization as failed
            pos_ = nullptr;
            end_ = nullptr;
            return false;
        }
        std::memcpy(buffer, pos_, length);
        pos_ += length;
        return true;
    }

    template <typename T>
    bool deserialize(T * value)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Value needs to be trivially copyable");
        return deserialize(value, sizeof(T));
    }

private:
    const char * pos_;
    const char * end_;
};


#endif  // TOOLS_SERDES_HPP_
