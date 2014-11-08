#pragma once

#include "Array.hpp"

#include "arc/memory/Allocator.hpp"
#include "arc/memory/util.hpp"

namespace arc
{
	template<typename T>
	inline void Array<T>::resize(arc::uint32 size)
	{
		if (size > m_capacity)
		{
			_grow(size);
		}
		if (size > m_size)
		{
			memory::util::init_elements<T>(&m_data[m_size], size - m_size);
		}
		else
		{
			memory::util::delete_elements<T>(&m_data[size], m_size - size);
		}
		m_size = size;
	}

	template<typename T>
	inline void Array<T>::resize(uint32 size, const T& init)
	{
		if (size > m_capacity)
		{
			_grow(size);
		}
		if (size > m_size)
		{
			auto dataT = &m_data[m_size];
			uint32 n = size - m_size;
			for (uint32 i = 0; i<n; i++)
			{
				// in place constructor
				new (&dataT[i]) T(init);
			}
		}
		else
		{
			memory::util::delete_elements<T>(&m_data[size], m_size - size);
		}
		m_size = size;
	}

	template<typename T> inline
	void Array<T>::reserve(uint32 size)
	{
		if (size > m_capacity)
		{
			_grow(size);
		}
	}

	template<typename T> inline
	void Array<T>::initialize(memory::Allocator* alloc, uint32 size)
	{
		if (is_initialized())
		{ 
			clear();
			m_allocator->free(m_data);
			m_capacity = m_size = 0;
			m_data = nullptr;
		}

		m_allocator = alloc;
		resize(size);
	}

	template<typename T> inline
	void Array<T>::trim()
	{
		if (m_capacity > m_size)
		{
			// allocate new memory
			auto newData = m_allocator->allocate(m_size*sizeof(T), alignof(T));
			// move to new memory
			memory::util::move_construct_elements<T>(m_data, newData, m_size);
			// free old memory
			memory::util::delete_elements<T>(m_data, m_size);
			m_allocator->free(m_data);
			// book keeping
			m_data = (T*)newData;
			m_capacity = m_size;
		}
	}


	template<typename T> inline
	void Array<T>::_grow(uint32 minCapacity)
	{
		// growth factor of 1.5, rounded up
		uint32 nextCapacity = m_capacity + (uint32)(0.5*(m_capacity + 1));
		// respect requested minimum capacity
		if (minCapacity > nextCapacity) nextCapacity = minCapacity;

		// allocate new memory
		auto newData = m_allocator->allocate(nextCapacity*sizeof(T), alignof(T));
		// move data to new memory
		memory::util::move_construct_elements<T>(m_data, newData, m_size);
		// delete old data and free
		memory::util::delete_elements<T>(m_data, m_size);
		m_allocator->free(m_data);
		// book keeping
		m_data = (T*)newData;
		m_capacity = nextCapacity;
	}

	template<typename T> inline
	uint32 Array<T>::size() const
	{
		return m_size;
	}

	template<typename T> inline
	bool Array<T>::empty() const
	{
		return m_size == 0;
	}

	template<typename T> inline
	T& Array<T>::operator[](uint32 idx)
	{
		return m_data[idx];
	}

	template<typename T> inline
	const T& Array<T>::operator[](uint32 idx) const
	{
		return m_data[idx];
	}

	template<typename T> inline
	Array<T>::Array(memory::Allocator& a, uint32 size)
		: m_allocator(&a)
	{
		resize(size);
	}

	template<typename T> inline
		Array<T>::Array(Array<T>&& other)
		: m_allocator(other.m_allocator)
		, m_data(other.m_data)
		, m_size(other.m_size)
		, m_capacity(other.m_capacity)
	{
		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;
	}

	template<typename T> inline
	Array<T>& Array<T>::operator=(Array<T>&& other)
	{
		m_allocator = other.m_allocator;
		m_data = other.m_data;
		m_size = other.m_size;
		m_capacity = other.m_capacity;

		other.m_data = nullptr;
		other.m_size = 0;
		other.m_capacity = 0;

		return *this;
	}

	template<typename T> inline
	void Array<T>::push_back(const T& value)
	{
		resize(m_size + 1);
		back() = value;
	}

	template<typename T> inline
	void Array<T>::push_back(T&& value)
	{
		resize(m_size + 1);
		back() = std::move(value);
	}

	template<typename T> inline
	void Array<T>::pop_back()
	{
		ARC_ASSERT(size() > 0, "Called pop_back() on empty Array");
		memory::util::delete_elements<T>(&back(), 1);
		m_size -= 1;
	}

	template<typename T> inline
	T& Array<T>::back()
	{
		return m_data[m_size - 1];
	}

	template<typename T> inline
	const T& Array<T>::back() const
	{
		return m_data[m_size - 1];
	}

	template<typename T> inline
	T* Array<T>::data()
	{
		return m_data;
	}

	template<typename T> inline
	const T* Array<T>::data() const
	{
		return m_data;
	}

	template<typename T>
	template<typename ...Args> inline
	void Array<T>::emplace_back(Args&& ...args)
	{
		resize(m_size + 1);
		new (&back()) T(std::forward<Args>(args)...);
	}

	template<typename T>
	T* begin(arc::Array<T>& a)
	{
		return a.data();
	}

	template<typename T>
	T* end(arc::Array<T>& a)
	{
		return a.data() + a.size();
	}

	template<typename T>
	const T* begin(const arc::Array<T>& a)
	{
		return a.data();
	}

	template<typename T>
	const T* end(const arc::Array<T>& a)
	{
		return a.data() + a.size();
	}
}