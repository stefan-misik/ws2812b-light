#ifndef SHARED_STORAGE_H_
#define SHARED_STORAGE_H_

#include <stddef.h>
#include <stdint.h>

inline void * operator new (size_t n, void * ptr)
{
    return ptr;
};


class SharedStorage
{
public:
    static const size_t SIZE = 16;

    template <typename T>
    void create()
    {
        static_assert(sizeof(T) <= SIZE, "Object does not fit");
        new (storage_) T();
        destructor_ = +[](void * obj) -> void
        {
            reinterpret_cast<T *>(obj)->~T();
        };
    }

    template <typename T>
    T * get()
    {
        return reinterpret_cast<T *>(storage_);
    }

    void destroy()
    {
        if (destructor_)
        {
            destructor_(storage_);
            destructor_ = nullptr;
        }
    }

private:
    void (* destructor_)(void *) = nullptr;
    char storage_[SIZE];
};




#endif /* SHARED_STORAGE_H_ */
