#pragma once

#include "entity/common.hpp"

namespace arc
{
	template < typename ComponentT, typename DataT >
	class SimpleComponentBackend
	{
	public:
		static typename ComponentT Get(uint32 index);
	public:
		static uint32 MaximumCount() { return s_maximum_count; }
		static uint32 ActiveCount()  { return s_active_count; }
	public:
		static bool Initialize(memory::Allocator* alloc, uint32 count);
		static uint32 Create(uint32 entity_index);
		static void   Return(uint32 component_index);
	public:
		static DataT*  s_component_data;
		static uint32*  s_entity_index_data;
		static uint32 s_maximum_count;
		static uint32 s_active_count;
		static memory::Allocator* s_alloc;
	};

	template<typename ComponentT, typename DataT>
	class SimpleComponent
	{
	public:
		using Backend = SimpleComponentBackend < ComponentT, DataT > ;
	public:
		bool valid() { return m_index != engine::INVALID_COMPONENT_INDEX; }
	protected:
		DataT* data();
		uint32 m_index = engine::INVALID_COMPONENT_INDEX; // index into the component array
	private:
		friend class Backend;
	};

	// implementation ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename ComponentT, typename DataT>
	DataT* SimpleComponent<ComponentT, DataT>::data()
	{
		ARC_ASSERT(valid(), "Trying to access invalid Component");
		return Backend::s_component_data + m_index;
	}

	template<typename ComponentT, typename DataT>
	/*static*/ typename ComponentT SimpleComponentBackend<ComponentT, DataT>::Get(uint32 index)
	{
		if (index >= s_active_count) index = engine::INVALID_COMPONENT_INDEX;
		ComponentT comp;
		comp.m_index = index;
		return comp;
	}

	template<typename ComponentT, typename DataT>
	/*static*/ uint32 SimpleComponentBackend<ComponentT, DataT>::s_maximum_count = 0;

	template<typename ComponentT, typename DataT>
	/*static*/ uint32 SimpleComponentBackend<ComponentT, DataT>::s_active_count = 0;

	template<typename ComponentT, typename DataT>
	/*static*/ memory::Allocator* SimpleComponentBackend<ComponentT, DataT>::s_alloc = nullptr;

	template<typename ComponentT, typename DataT>
	/*static*/ DataT* SimpleComponentBackend<ComponentT, DataT>::s_component_data = nullptr;

	template<typename ComponentT, typename DataT>
	/*static*/ uint32* SimpleComponentBackend<ComponentT, DataT>::s_entity_index_data = nullptr;

	template<typename ComponentT, typename DataT>
	/*static*/ uint32 SimpleComponentBackend<ComponentT, DataT>::Create(uint32 entity_index)
	{
		if (s_active_count >= s_maximum_count) return engine::INVALID_COMPONENT_INDEX;

		// get next free component
		uint32 component_index = s_active_count++;

		// link component to entity
		s_entity_index_data[component_index] = entity_index;

		return component_index;
	}

	template<typename ComponentT, typename DataT>
	/*static*/ void SimpleComponentBackend<ComponentT, DataT>::Return(uint32 component_index)
	{
		ARC_ASSERT(component_index < s_active_count, "Index out of bounds.");

		s_active_count -= 1;

		// swap components
		s_component_data[component_index] = s_component_data[s_active_count];

		// update swapped component linking
		auto type_id = ManualTypeId<engine::ComponentIdContext, ComponentT>::Value();
		engine::_relink_component_to_entity(s_entity_index_data[component_index], type_id, component_index);
	}

	template<typename ComponentT, typename DataT>
	/*static*/ bool SimpleComponentBackend<ComponentT, DataT>::Initialize(memory::Allocator* alloc, uint32 count)
	{
		if (s_entity_index_data != nullptr) return false; // already initialized

		s_maximum_count = count;
		s_active_count = 0;
		s_alloc = alloc;
		s_component_data = (DataT*)alloc->allocate(sizeof(DataT)*count);
		s_entity_index_data = (uint32*)alloc->allocate(sizeof(uint32)*count);

		return true;
	}

} // namespace arc