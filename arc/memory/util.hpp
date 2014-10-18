#pragma once

#include <utility>           // std::move
#include <type_traits>		 // std::is_trivial 
#include <cstring>           // std::memcpy

namespace arc { namespace memory { namespace util 
{

    template<typename T> inline
    void move_elements(void* from, void* to, uint32 n)
    {
        if (std::is_trivial<T>::value)
        {
            std::memcpy(to, from, n*sizeof(T));
        }
        else
        {
            auto fromT = static_cast<T*>(from);
            auto toT = static_cast<T*>(to);
            for (uint32 i=0; i<n; i++)
            {
                toT[i] = std::move(fromT[i]);
            }
        }
    }

    template<typename T> inline
    void move_construct_elements(void* from, void* to, uint32 n)
    {
        if (std::is_trivial<T>::value)
        {
            std::memcpy(to, from, n*sizeof(T));
        }
        else
        {
            auto fromT = static_cast<T*>(from);
            auto toT = static_cast<T*>(to);
            for (uint32 i=0; i<n; i++)
            {
                // in place constructor
                new (&toT[i]) T(std::move(fromT[i]));
            }
        }
    }

    template<typename T> inline
    void copy_elements(void* from, void* to, uint32 n)
    {
        if (std::is_trivial<T>::value)
        {
            std::memcpy(to, from, n*sizeof(T));
        }
        else
        {
            auto fromT = static_cast<T*>(from);
            auto toT = static_cast<T*>(to);
            for (uint32 i=0; i<n; i++)
            {
                toT[i] = fromT[i];
            }
        }
    }

    template<typename T> inline
    void delete_elements(void* data, uint32 n)
    {
        if (std::is_trivial<T>::value)
        {
            // do nothing
        }
        else
        {
            auto dataT = static_cast<T*>(data);
            for (uint32 i=0; i<n; i++)
            {
                dataT[i].~T();
            }
        }
    }

    template<typename T> inline 
    void init_elements(void* data, uint32 n)
    {
        if (std::is_trivial<T>::value)
        {
            // do nothing
        }
        else
        {
            auto dataT = static_cast<T*>(data);
            for (uint32 i=0; i<n; i++)
            {
                // in place constructor
                new (&dataT[i]) T();
            }
        }
    }

	template<typename W, typename T> inline
	T forward_align(T value, W alignment)
	{
		int64 v = value+alignment-1;
		int64 f = v / alignment;
		return static_cast<T>(f * alignment);

		/*
			uint32 mod = begin % alignment;
			if (mod == 0) return begin;
			else return begin + alignment - mod;
		*/
	}

	template<typename W> inline
	void* forward_align_ptr(void* value, W alignment)
	{
		return (void*)((1 + ((size_t)value - 1) / alignment) * alignment);

		/*
		uint32 mod = begin % alignment;
		if (mod == 0) return begin;
		else return begin + alignment - mod;
		*/
	}

	template<typename W> inline
	void* ptr_add(void* ptr, W v)
	{
		return ((char*)ptr) + v;
	}

	inline void copy(void* from, void* to, uint32 byte_count)
	{
		std::memcpy(to, from, byte_count);
	}

}}} // namespace arc::memory::util
