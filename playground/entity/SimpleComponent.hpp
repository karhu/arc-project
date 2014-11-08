#pragma once

#include "entity.hpp"

#include "arc/collections/HashMap.inl"
#include "arc/collections/Array.inl"

namespace arc { namespace entity {

	template<typename ChildT, typename DataT>
	class SimpleComponent;

	template<typename HandleType, typename DataType>
	class SimpleComponentBackend : public ComponentBackend
	{
	public:
		using DataT = DataType;
		using HandleT = HandleType;
	public:
		SimpleComponentBackend(memory::Allocator* alloc, uint32 capacity);
	public:
		HandleT add_component(entity::Handle h);
		HandleT get_component(entity::Handle h);
	public:
		template<typename F>
		void update_all(F function);
	protected:
		HashMap<int32>			m_mapping;
		Array<DataT>			m_data;
		Array<entity::Handle>	m_entities;
		uint32_t				m_count;
	protected:
		friend class SimpleComponent<HandleT,DataT>;
	};

	template<typename ChildT, typename DataT>
	class SimpleComponent 
	{
	public:
		bool valid() const             { return m_backend != nullptr; }
		entity::Handle entity() const { ARC_ASSERT(valid(), "invalid component handle"); return m_backend->m_entities[m_index]; }
	protected:
		using Data = DataT;
		using Handle = ChildT;
		using Backend = SimpleComponentBackend<Handle,Data>;
	protected:
		DataT& data()             { ARC_ASSERT(valid(), "invalid component handle"); return m_backend->m_data[m_index]; }
		const DataT& data() const { ARC_ASSERT(valid(), "invalid component handle"); return m_backend->m_data[m_index]; }
		Backend& backend()             { ARC_ASSERT(valid(), "invalid component handle"); return m_backend; }
		const Backend& backend() const { ARC_ASSERT(valid(), "invalid component handle"); return m_backend; }
	private:
		Backend* m_backend = nullptr;
		uint32_t m_index = 0;
	protected:
		template<typename T, typename R>
		friend class SimpleComponentBackend;
		friend class entity::Context;
	};

	template< typename HandleType, typename DataType>
	SimpleComponentBackend<HandleType, DataType>::SimpleComponentBackend(memory::Allocator* alloc, uint32 capacity)
		: m_mapping(*alloc), m_data(*alloc, capacity), m_entities(*alloc, capacity), m_count(0)
	{}

	template< typename HandleType, typename DataType>
	HandleType SimpleComponentBackend<HandleType, DataType>::add_component(entity::Handle h)
	{
		// we only support a single instance of a component per entity
		if (m_mapping.contains(h.index())) return HandleT();
		// the maximum number of components was reached
		if (m_count == m_data.size()) return HandleT();

		// allocating and book keeping
		uint32 index = m_count;
		m_count += 1;
		m_entities[index] = h;
		m_mapping.set(h.index(),index);

		HandleT ch;
		ch.m_backend = this;
		ch.m_index = index;
		return ch;
	}

	template< typename HandleType, typename DataType>
	HandleType SimpleComponentBackend<HandleType, DataType>::get_component(entity::Handle h)
	{
		auto entry = m_mapping.lookup(h.index());
		if (!entry) return HandleT();

		HandleT ch;
		ch.m_backend = this;
		ch.m_index = entry->value();
		return ch;
	}


	template<typename HandleType, typename DataType>
	template<typename F>
	void SimpleComponentBackend<HandleType, DataType>::update_all(F function)
	{
		HandleT component;
		component.m_backend = this;

		for (uint32_t i = 0; i < m_count; i++)
		{
			component.m_index = i;
			function(component);
		}
	}
}}

