/**
 * @file
 */

#ifndef TOOLS_BIP_BUFFER_CONTROLLER_HPP_
#define TOOLS_BIP_BUFFER_CONTROLLER_HPP_

#include <cstddef>

#include <utility>
#include <concepts>
#include <algorithm>


/**
 * @brief Tracks reservations in a bipartite circular buffer
 *
 * The caller owns the backing storage and supplies its capacity to operations
 * that need it. One element is always kept unused to distinguish a full
 * buffer from an empty buffer.
 *
 * @tparam SZ_T Unsigned type used for buffer offsets and lengths
 */
template <std::unsigned_integral SZ_T = std::size_t>
class BipBufferController
{
public:
    /** @brief Type used for buffer offsets and lengths */
    using SizeType = SZ_T;

    /**
     * @brief Creates an empty controller for a buffer of the given capacity
     *
     * @param capacity Capacity of the associated backing buffer
     */
    BipBufferController(SizeType capacity):
        active_capacity_(capacity)
    { }

    /**
     * @brief Checks whether no committed data is available
     *
     * @return `true` when the buffer is empty
     */
    bool isEmpty() const
    {
        return head_ == tail_;
    }

    /**
     * @brief Discards all data and resets the controller
     *
     * @param capacity Capacity of the associated backing buffer
     */
    void clear(SizeType capacity)
    {
        head_ = 0;
        tail_ = 0;
        active_capacity_ = capacity;
    }

    /**
     * @brief Reserves one contiguous writable region
     *
     * The returned region can be shorter than requested when insufficient
     * contiguous space is available.
     *
     * @param requested_size Preferred number of elements to reserve
     * @param capacity Capacity of the associated backing buffer
     * @return Pair of the region offset and its reserved length
     */
    std::pair<SizeType, SizeType> reserve(SizeType requested_size, SizeType capacity) const
    {
        const SizeType space_ahead = spaceAhead(capacity);
        if (space_ahead < requested_size)
        {
            const SizeType space_behind = spaceBehind();
            if (space_behind > space_ahead)
                return {0, std::min<SizeType>(space_behind, requested_size)};
        }
        return {head_, std::min<SizeType>(space_ahead, requested_size)};
    }

    /**
     * @brief Makes written elements at the current head available to readers
     *
     * @param commit_offset Offset returned from @ref reserve() for the region to commit
     * @param commit_size Number of elements to commit
     * @param capacity Capacity of the associated backing buffer
     *
     * @return Success
     */
    bool commit(SizeType commit_offset, SizeType commit_size, SizeType capacity)
    {
        if (0 == commit_offset && 0 != head_)
        {
            if (commit_size > spaceBehind())
                return false;

            active_capacity_ = head_;
            head_ = 0;
        }
        else
        {
            if (commit_size > spaceAhead(capacity))
                return false;
        }

        head_ += commit_size;
        return true;
    }

    /**
     * @brief Gets the next contiguous readable region
     *
     * @return Pair of the region offset and its available length
     */
    std::pair<SizeType, SizeType> getContiguousBlock() const
    {
        if (isContiguous())
            return {tail_, head_ - tail_};
        else
            return {tail_, active_capacity_ - tail_};
    }

    /**
     * @brief Releases elements from the current readable region
     *
     * @param size_to_decommit Number of elements to release
     * @param capacity Capacity of the associated backing buffer
     */
    void decommit(SizeType size_to_decommit, SizeType capacity)
    {
        const SizeType new_tail = tail_ + size_to_decommit;
        if (isContiguous())
        {
            if (new_tail < head_)
                tail_ = new_tail;
            else
            {
                head_ = 0;
                tail_ = 0;
            }
        }
        else
        {
            if (new_tail < active_capacity_)
                tail_ = new_tail;
            else
            {
                tail_ = 0;
                active_capacity_ = capacity;
            }
        }
    }

private:
    SizeType head_ = 0;
    SizeType tail_ = 0;
    SizeType active_capacity_ = 0;

    /**
     * @brief Checks whether committed data has not wrapped around the buffer
     *
     * @return `true` when the head is at or after the tail
     */
    bool isContiguous() const
    {
        return head_ >= tail_;
    }

    /**
     * @brief Calculates contiguous writable space at the head
     *
     * @param capacity Capacity of the associated backing buffer
     * @return Number of writable elements ahead of the head
     */
    SizeType spaceAhead(SizeType capacity) const
    {
        if (isContiguous())
            return capacity - head_ - (0 == tail_ ? 1 : 0);
        else
            return (tail_ - head_ - 1);
    }

    /**
     * @brief Calculates writable space before the tail after wrapping
     *
     * @return Number of writable elements before the tail
     */
    SizeType spaceBehind() const
    {
        if (isContiguous())
            return 0 == tail_ ? 0 : (tail_ - 1);
        else
            return 0;
    }
};


#endif  // TOOLS_BIP_BUFFER_CONTROLLER_HPP_
