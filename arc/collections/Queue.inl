#pragma once

#include "Queue.hpp"

#include "arc/core.hpp"
#include "arc/memory/Allocator.hpp"
#include "arc/memory/util.hpp"
#include <algorithm>

namespace arc
{

	template<typename T>
	uint32 Queue<T>::last_index() const
	{
		return (m_first + m_size - 1) % m_capacity;
	}

	template<typename T>
	uint32 Queue<T>::size() const
	{
		return m_size;
	}

	template<typename T>
	uint32 Queue<T>::capacity() const
	{
		return m_capacity;
	}

	template<typename T>
	bool Queue<T>::empty() const
	{
		return m_size == 0;
	}

	template<typename T>
	T Queue<T>::pop_front()
	{
		ARC_ASSERT(size() > 0, "Called pop_front() on empty queue.");

		T tmp;
		memory::util::move_elements<T>(&front(),&tmp, 1);
		memory::util::delete_elements<T>(&front(),1);
		m_first = (m_first + 1) % m_capacity;
		m_size -= 1;
		return tmp;
	}

	template<typename T>
	T Queue<T>::pop_back()
	{
		ARC_ASSERT(size() > 0, "Called pop_back() on empty queue.");

		T tmp;
		memory::util::move_elements<T>(&front(), &tmp, 1);
		collection_util::delete_elements<T>(&back(),1);
		m_size -= 1;
		return tmp;
	}

	template<typename T>
	void Queue<T>::remove_front()
	{
		ARC_ASSERT(size() > 0, "Called remove_front() on empty queue.");

		memory::util::delete_elements<T>(&front(), 1);
		m_first = (m_first + 1) % m_capacity;
		m_size -= 1;
	}

	template<typename T>
	void Queue<T>::remove_back()
	{
		ARC_ASSERT(size() > 0, "Called remove_back() on empty queue.");

		collection_util::delete_elements<T>(&back(), 1);
		m_size -= 1;
	}

	template<typename T>
	T& Queue<T>::front()
	{
		ARC_ASSERT(size() > 0, "Called front() on empty queue.");
		return m_data[m_first];
	}

	template<typename T>
	T& Queue<T>::back()
	{
		ARC_ASSERT(size() > 0, "Called back() on empty queue.");
		auto last = last_index();
		return m_data[last];
	}

	template<typename T>
	void Queue<T>::grow(uint32 additionalm_capacity)
	{
		namespace cu = arc::memory::util;

		uint32 newm_capacity = m_capacity + additionalm_capacity;
		T* newm_data = (T*)m_alloc->allocate(newm_capacity*sizeof(T),alignof(T));

		uint32 frontm_size = std::min(m_size,m_capacity - m_first);
		uint32 backm_size = m_size - frontm_size;

		cu::move_construct_elements<T>(m_data+m_first,newm_data,frontm_size);
		cu::delete_elements<T>(m_data+m_first,frontm_size);

		cu::move_construct_elements<T>(m_data,newm_data+frontm_size,backm_size);
		cu::delete_elements<T>(m_data,backm_size);

		m_alloc->free(m_data);

		m_capacity = newm_capacity;
		m_first = 0;
		m_data = newm_data;
	}

	template<typename T>
	void Queue<T>::push_front(const T& value)
	{
		if (size() == capacity()) grow(0.5f*m_capacity+8);

		m_size += 1;
		m_first = (m_first+m_capacity-1) % m_capacity;
		new (&m_data[m_first]) T(value);
	}

	template<typename T>
	void Queue<T>::push_back(const T& value)
	{
		if (size() == capacity())
		{
			float inc = 0.5f*m_capacity + 8;
			grow((uint32)inc);
		}

		m_size += 1;
		new (&m_data[last_index()]) T(value);
	}

	template<typename T>
	Queue<T>::Queue(memory::Allocator* alloc, uint32 capacity)
		: m_alloc(alloc), m_capacity(capacity)
	{
		m_data = (T*)alloc.allocate(capacity*sizeof(T),alignof(T));
	}

	template<typename T>
	void Queue<T>::initialize(memory::Allocator* alloc, uint32 capacity = 8)
	{
		finalize();

		m_alloc = alloc;
		m_capacity = capacity;
		m_data = (T*)m_alloc->allocate(capacity*sizeof(T), alignof(T));
	}

	template<typename T>
	void Queue<T>::finalize()
	{
		if (is_initialized())
		{
			namespace cu = memory::util;
			uint32 frontm_size = std::min(m_size, m_capacity - m_first);
			uint32 backm_size = m_size - frontm_size;

			cu::delete_elements<T>(m_data + m_first, frontm_size);
			cu::delete_elements<T>(m_data, backm_size);
			m_alloc->free(m_data);
		}
	}

	template<typename T>
	bool Queue<T>::is_initialized()
	{
		return m_alloc != nullptr;
	}

} // namespace arc
