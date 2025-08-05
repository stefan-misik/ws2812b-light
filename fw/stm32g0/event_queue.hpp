/**
 * @file
 */

#ifndef EVENT_QUEUE_HPP_
#define EVENT_QUEUE_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>

#include <type_traits>


/**
 * @brief Simple queue object allowing to store events in FIFO fashion
 */
class EventQueue
{
public:
    enum class EventType
    {
        NONE,
        KEY_EVENT,
    };

    class Event
    {
    public:
        Event() = default;

        template <typename T>
        void set(EventType type, T && value)
        {
            type_ = type;
            checkType<T>();
            *reinterpret_cast<T *>(&param_) = value;
        }

        EventType type() const { return type_; }

        template <typename T>
        const T & param() const
        {
            checkType<T>();
            return *reinterpret_cast<const T *>(&param_);
        }

    private:
        struct StorageType
        {
            alignas(std::uintptr_t)
            char v[sizeof(std::uintptr_t)];
        };

        EventType type_;
        StorageType param_;

        template <typename T>
        static void checkType()
        {
            static_assert(
                    std::is_trivially_copyable<T>::value &&
                    std::is_trivially_destructible<T>::value &&
                    sizeof(T) <= sizeof(StorageType) && alignof(T) <= alignof(StorageType),
                    "Type needs to be trivially copy-able and of appropriate size");
        }
    };

    static const inline std::size_t CAPACITY = 32;

    template <typename T>
    bool pushEvent(EventType type, T && param)
    {
        std::size_t next_head = (CAPACITY - 1) != head_ ? head_ + 1 : 0;
        if (next_head == tail_)
            return false;
        events_[head_].set(type, std::forward<T>(param));
        head_ = next_head;
        return true;
    }

    std::pair<const Event *, std::size_t> peek() const
    {
        if (isContiguous())
            return {events_ + tail_, head_ - tail_};
        else
            return {events_ + tail_, CAPACITY - tail_};
    }

    void release(std::size_t size)
    {
        std::size_t next_tail = tail_ + size;
        tail_ = (CAPACITY != next_tail) ? next_tail : 0;
    }

    void discard()
    {
        head_ = 0;
        tail_ = 0;
    }

private:
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    Event events_[CAPACITY];

    bool isContiguous() const { return head_ >= tail_; }
};



#endif  // EVENT_QUEUE_HPP_
