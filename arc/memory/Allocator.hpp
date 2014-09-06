#pragma once

#include "arc/core/numeric_types.hpp"

namespace arc
{
namespace memory
{

class Allocator
{
public:
	Allocator() {}
	virtual ~Allocator() {}

public:
    virtual void* allocate(uint64 size, uint32 align = 4) = 0;
    virtual void  free(void* data) = 0;

    template <typename T, typename ...Args> inline
    T* create(Args&& ...args)
    {
        void* ptr = allocate(sizeof(T),alignof(T));
        // inplace constructor
        return new (ptr) T(std::forward<Args>(args)...);
    }

    template <typename T, typename ...Args> inline
    T* create_n(uint64 n, Args&& ...args)
    {
        void* ptr = allocate(n*sizeof(T),alignof(T));
        // inplace constructor
		for (uint64 i = 0; i < n; i++)
		{
			void* p = (T*)ptr + i;
			new (p)T(std::forward<Args>(args)...);
		}
		return (T*)ptr;
    }

    template <typename T> inline
    void destroy(T* ptr)
    {
        ptr->~T();
        free(ptr);
    }

    template <typename T> inline
    void destroy_n(T* ptr, uint64 n)
    {
        for (uint64 i=0; i<n; i++)
        {
            ptr[i].~T();
        }
        free(ptr);
    }
};

class Mallocator final : public Allocator
{
public:
    void* allocate(uint64 size, uint32 align = 4) override;
    void  free(void* data) override;
};

class DummyAllocator final : public Allocator
{
	void* allocate(uint64 size, uint32 align = 4) override;
	void  free(void* data) override;
};

extern DummyAllocator g_dummy_allocator;


}} // namespaces


namespace arc
{
    template <typename T, typename ...Args>
    T* create(arc::memory::Allocator& alloc, Args&& ...args)
    {
        void* ptr = alloc.allocate(sizeof(T),alignof(T));
        // inplace constructor
        return new (ptr) T(std::forward<Args>(args)...);
    }

    template <typename T, typename ...Args>
    T* create_n(arc::memory::Allocator& alloc, uint32 n, Args&& ...args)
    {
        void* ptr = alloc.allocate(n*sizeof(T),alignof(T));
        // inplace constructor
        return new (ptr) T(std::forward<Args>(args)...);
    }
}

