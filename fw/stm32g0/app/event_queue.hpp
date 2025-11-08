/**
 * @file
 */

#ifndef APP_EVENT_QUEUE_HPP_
#define APP_EVENT_QUEUE_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <new>
#include <type_traits>
#include <concepts>


/**
 * @brief List of known event types
 */
enum class EventType
{
    NONE,
    KEY_EVENT,
};


/**
 * @brief Object used to store event parameter value
 */
struct EventParameterStorageType
{
    alignas(std::uintptr_t)
    char v[sizeof(std::uintptr_t)];
};


/**
 * @brief Concept of event parameter objects
 */
template <typename T>
concept EventParameter =
    std::is_trivially_copyable<T>::value &&
    std::is_trivially_destructible<T>::value &&
    sizeof(T) <= sizeof(EventParameterStorageType) &&
    alignof(T) <= alignof(EventParameterStorageType) &&
    requires()
{
    { T::TYPE } -> std::convertible_to<EventType>;
};


/**
 * @brief Simple queue object allowing to store events in FIFO fashion
 */
class EventQueue
{
public:
    class Event
    {
    public:
        Event() = default;

        template <EventParameter T, typename... Ts>
        void set(Ts &&... args)
        {
            new (&param_) T(std::forward<Ts>(args)...);
            type_ = T::TYPE;
        }

        EventType type() const { return type_; }

        template <EventParameter T>
        const T & param() const
        {
            return *reinterpret_cast<const T *>(&param_);
        }

    private:
        EventType type_;
        EventParameterStorageType param_;
    };

    static const inline std::size_t CAPACITY = 32;

    template <EventParameter T, typename... Ts>
    bool pushEvent(Ts &&... args)
    {
        Event * const ev = allocate();
        if (nullptr == ev)
            return false;
        ev->template set<T>(std::forward<Ts>(args)...);
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

    Event * allocate()
    {
        std::size_t next_head = (CAPACITY - 1) != head_ ? head_ + 1 : 0;
        if (next_head == tail_)
            return nullptr;
        Event * ev = events_ + head_;
        head_ = next_head;
        return ev;
    }
};



#endif  // APP_EVENT_QUEUE_HPP_
