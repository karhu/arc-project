#pragma once

#include "entity2.hpp"

#include "arc/collections/HashMap.inl"
#include "arc/collections/Array.inl"

namespace arc { namespace entity2 {


	template<typename DataT, typename HandleT>
	class SimpleComponent : public ComponentBase
	{
	public:
		using Data = DataT;
		using Handle = HandleT;
	public:
		SimpleComponent(memory::Allocator* alloc, uint32 capacity);
	public:
		Handle add_component(entity2::Handle h);
		Handle get_component(entity2::Handle h);
	protected:
		HashMap<int32>			m_mapping;
		Array<Data>				m_data;
		Array<entity2::Handle>	m_entities;
		uint32_t				m_count;
	};

	template<typename DataT>
	class SimpleComponentHandle 
	{
	public:
		DataT& data()             { ARC_ASSERT(valid(), "invalid component handle"); return *m_data; }
		const DataT& data() const { ARC_ASSERT(valid(), "invalid component handle"); return *m_data; }
		bool valid() const        { return m_data != nullptr; }
	protected:
		DataT* m_data = nullptr;
	protected:
		template<typename T, typename R>
		friend class SimpleComponent;
	};

	template<typename DataT, typename HandleT>
	SimpleComponent<DataT, HandleT>::SimpleComponent(memory::Allocator* alloc, uint32 capacity)
		: m_mapping(*alloc), m_data(*alloc, capacity), m_entities(*alloc, capacity), m_count(0)
	{}

	template<typename DataT, typename HandleT>
	HandleT SimpleComponent<DataT, HandleT>::add_component(entity2::Handle h)
	{
		// we only support a single instance of a component per entity
		if (m_mapping.contains(h.index())) return Handle();
		// the maximum number of components was reached
		if (m_count == m_data.size()) return Handle();

		// allocating and book keeping
		uint32 index = m_count;
		m_count += 1;
		m_entities[index] = h;
		m_mapping.set(h.index(),index);

		Handle ch;
		ch.m_data = &m_data[index];
		return ch;
	}

	template<typename DataT, typename HandleT>
	HandleT SimpleComponent<DataT, HandleT>::get_component(entity2::Handle h)
	{
		auto entry = m_mapping.lookup(h.index());
		if (!entry) return Handle();

		Handle ch;
		ch.m_data = &m_data[entry->value()];
		return ch;
	}




#if 0
	template<typename ComponentT, typename DataT>
	class SimpleComponentBackend : public ComponentBackend
	{
	public:
		SimpleComponentBackend(memory::Allocator* alloc, uint32 initial_capacity);
	public:
		ComponentT add_component(entity2::Handle h);
		ComponentT get_component(entity2::Handle h);
	private:
		HashMap<int32> m_mapping;
		Array<DataT> m_data;
		Array<entity2::Handle> m_entities;
	};

	template<typename ComponentT, typename DataT>
	class SimpleComponent
	{
	public:
		bool valid() const;
	protected:
		DataT* data();
		const DataT* data() const;
	protected:
		using Backend = SimpleComponentBackend < ComponentT, DataT >;
	private:
		DataT* m_data = nullptr;
	private:
		friend class SimpleComponentBackend < ComponentT, DataT >;
		friend class entity2::Context;
	};

	// Simple Component Template Implementation ////////////////////////////////////////////////////////////////////

	template<typename ComponentT, typename DataT>
	bool SimpleComponent<ComponentT, DataT>::valid() const { return m_data != nullptr; }

	template<typename ComponentT, typename DataT>
	DataT* SimpleComponent<ComponentT, DataT>::data() { ARC_ASSERT(valid(), "Invalid Component Handle"); return m_data; }

	template<typename ComponentT, typename DataT>
	const DataT* SimpleComponent<ComponentT, DataT>::data() const { ARC_ASSERT(valid(), "Invalid Component Handle"); return m_data; }

	// Simple Component Backend Template Implementation ////////////////////////////////////////////////////////////

	template<typename CT, typename DT>
	SimpleComponentBackend<CT, DT>::SimpleComponentBackend(memory::Allocator* alloc, uint32 initial_capacity)
		: m_data(*alloc), m_mapping(*alloc), m_entities(*alloc)
	{
		m_data.reserve(initial_capacity);
	}

	template<typename CT, typename DT>
	CT SimpleComponentBackend<CT, DT>::add_component(entity2::Handle h)
	{
		if (m_mapping.contains(h.index()))
		{
			return CT();
		}

		uint32 index = m_data.size();
		m_data.push_back();
		m_entities.push_back(h);

		m_mapping.set(h.index(),index);

		CT ct;
		ct.m_data = &m_data.back();
		return ct;
	}

	template<typename CT, typename DT>
	CT SimpleComponentBackend<CT, DT>::get_component(entity2::Handle h)
	{
		auto entry = m_mapping.lookup(h.index());
		if (!entry) return CT();

		CT ct;
		ct.m_data = &m_data[entry->value()];
		return ct;
	}
#endif
}}

