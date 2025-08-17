/**
 * @file
 */

#ifndef TOOLS_LIST_HPP_
#define TOOLS_LIST_HPP_


/**
 * @brief Simple linked list
 */
class List
{
public:
    class Node
    {
    protected:
        Node * next_ = nullptr;

        friend List;
    };

    /**
     * @brief Push a node into the list
     *
     * @param node Node to add to the end of the list
     */
    void push(Node * node)
    {
        node->next_ = nullptr;
        if (nullptr == first_)
        {
            first_ = node;
            last_ = node;
        }
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
     * @return Oldest node in the list
     * @retval nullptr List is empty
     */
    Node * peek() const
    {
        return first_;
    }

    /**
     * @brief Get an item from the list
     *
     * @return Oldest node in the list
     * @retval nullptr List is empty
     */
    Node * pop()
    {
        Node * const node = first_;
        if (nullptr != node)
            first_ = node->next_;
        return node;
    }

private:
    /** @brief First (oldest) item in the list */
    Node * first_ = nullptr;
    /** @brief Last (newest) item in the list, only valid when @ref first_ is not `nullptr` */
    Node * last_;
};


#endif  // TOOLS_LIST_HPP_
