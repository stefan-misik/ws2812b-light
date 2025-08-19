/**
 * @file
 */

#ifndef TOOLS_ATOMIC_LIST_HPP_
#define TOOLS_ATOMIC_LIST_HPP_

#include <atomic>

/**
 * @brief Atomic list designed for SINGLE consumer and SINGLE producer
 *
 * @tparam T Type of the item in the list
 */
template <typename T>
class AtomicList
{
public:
    using ListItem = T;

    class Node
    {
    public:
        using ListItem = T;

    protected:
        std::atomic<Node *> next_ = nullptr;

        friend AtomicList;
    };

    /**
     * @brief Push a node into the list
     *
     * @attention Only one thread is allowed to call this method
     *
     * @param node Node to add to the end of the list
     */
    void push(Node * node)
    {
        node->next_ = nullptr;
        Node * expected = nullptr;
        if (first_.compare_exchange_strong(expected, node, std::memory_order_release))
            last_ = node;
        else
        {
            Node * const last = last_;
            last_ = node;
            last->next_ = node;
        }
    }

    /**
     * @brief Peek the node from the list
     *
     * @attention Only one thread is allowed to call this method (same as @ref pop() )
     *
     * @return Oldest node in the list
     * @retval nullptr List is empty
     */
    const Node * peek() const
    {
        return first_;
    }

    /**
     * @brief Get an item from the list
     *
     * @attention Only one thread is allowed to call this method (same as @ref peek() )
     *
     * @return Oldest node in the list
     * @retval nullptr List is empty
     */
    Node * pop()
    {
        Node * const node = first_.load(std::memory_order_acquire);
        if (nullptr != node)
            first_ = node->next_;
        return node;
    }

private:
    std::atomic<Node *> first_ = nullptr;
    Node * last_;  // Only producer updates this variable

    static_assert(std::atomic<Node *>::is_always_lock_free, "Pointer needs to be always lock-free atomic");
};

#endif  // TOOLS_ATOMIC_LIST_HPP_
