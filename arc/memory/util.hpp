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

}}} // namespace arc::memory::util
