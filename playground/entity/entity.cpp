#include "entity.hpp"

#include "arc/memory/static_allocator.hpp"
#include "arc/collections/Array.inl"

namespace arc { namespace entity {

	Context::Context(memory::Allocator* alloc, uint32 initial_capacity)
		:  m_allocator(alloc)
	{
		m_index_pool.initialize(alloc, initial_capacity, 256, std::numeric_limits<uint32>::max());

		for (auto& ptr : m_component_backends) ptr = nullptr;
	}

	Context::~Context()
	{
		for (auto ptr : m_component_backends) if (ptr) m_allocator->destroy<ComponentBackend>(ptr);
	}

	Handle Context::create_entity()
	{
		Handle h;
		h.m_index = m_index_pool.create();
		return h;
	}

	bool Context::destroy_entity(Handle h)
	{
		if (!m_index_pool.valid(h.m_index)) return false;
		ARC_NOT_IMPLEMENTED;
		m_index_pool.release(h.m_index);
		return true;
	}

	bool Context::valid(Handle h)
	{
		return m_index_pool.valid(h.index());
	}

}}