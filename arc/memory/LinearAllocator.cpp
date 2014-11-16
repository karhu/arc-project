#include "LinearAllocator.hpp"

#include "arc/memory/util.hpp"

namespace arc { namespace memory {

	LinearAllocator::LinearAllocator(memory::Allocator* parent, uint32 size)
		: m_parent_alloc(parent)
	{
		m_begin = (char*)m_parent_alloc->allocate(size);
		if (m_begin == nullptr) size = 0;

		m_current = m_begin;
		m_end = m_begin + size;
	}

	LinearAllocator::~LinearAllocator() 
	{ 
		if (m_parent_alloc != nullptr)
		{
			m_parent_alloc->free(m_begin);
			m_begin = m_current = m_end = nullptr;
			m_parent_alloc = nullptr;
		}
	}

	void* LinearAllocator::allocate(uint64 size, uint32 align)
	{
		auto aligned = (char*)memory::util::forward_align((size_t)m_current, align);
		auto new_front = aligned + size;

		// out of memory
		if (new_front >= m_end) return nullptr;

		m_current = new_front;
		return aligned;
	}

	void LinearAllocator::free(void* data)
	{}

	void LinearAllocator::reset()
	{
		m_current = m_begin;
	}

}}