#if 0
#pragma once

#include "entity.hpp"

namespace arc
{
	template < typename ComponentT, typename DataT, typename CallbackT >
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
		static void Return(uint32 component_index);
	public:
		static void RemovalCallback(Slice<uint32> indices);
	public:
		static void RegisterCallbacks() { CallbackT::Register(); }
	public:
		static DataT*  s_component_data;
		static uint32*  s_entity_index_data;
		static uint32 s_maximum_count;
		static uint32 s_active_count;
		static memory::Allocator* s_alloc;
	};

	template<typename ComponentT, typename DataT, typename CallbackT>
	class SimpleComponent
	{
	public:
		using Backend = SimpleComponentBackend < ComponentT, DataT, CallbackT >;
	public:
		bool valid() const { return m_index != entity::INVALID_COMPONENT_INDEX; }
	protected:
		DataT* data();
		const DataT* data() const;
		uint32 m_index = entity::INVALID_COMPONENT_INDEX; // index into the component array
	private:
		friend class Backend;
	};

	// implementation ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#undef  TEMPLATE_IMPLEMENTATION
	#define TEMPLATE_IMPLEMENTATION(T)	\
		template<typename ComponentT, typename DataT, typename CallbackT> \
		T SimpleComponent<ComponentT, DataT, CallbackT>::					

	TEMPLATE_IMPLEMENTATION(DataT*) data()
	{
		ARC_ASSERT(valid(), "Trying to access invalid Component");
		return Backend::s_component_data + m_index;
	}

	TEMPLATE_IMPLEMENTATION(const DataT*) data() const
	{
		ARC_ASSERT(valid(), "Trying to access invalid Component");
		return Backend::s_component_data + m_index;
	}

	#undef  TEMPLATE_IMPLEMENTATION
	#define TEMPLATE_IMPLEMENTATION(T)	\
		template<typename ComponentT, typename DataT, typename CallbackT> \
		T SimpleComponentBackend<ComponentT, DataT, CallbackT>::	

	TEMPLATE_IMPLEMENTATION(typename ComponentT) Get(uint32 index)
	{
		if (index >= s_active_count) index = entity::INVALID_COMPONENT_INDEX;
		ComponentT comp;
		comp.m_index = index;
		return comp;
	}

	TEMPLATE_IMPLEMENTATION(uint32) s_maximum_count = 0;

	TEMPLATE_IMPLEMENTATION(uint32) s_active_count = 0;

	TEMPLATE_IMPLEMENTATION(memory::Allocator*) s_alloc = nullptr;

	TEMPLATE_IMPLEMENTATION(DataT*) s_component_data = nullptr;

	TEMPLATE_IMPLEMENTATION(uint32*) s_entity_index_data = nullptr;

	TEMPLATE_IMPLEMENTATION(uint32) Create(uint32 entity_index)
	{
		if (s_active_count >= s_maximum_count) return entity::INVALID_COMPONENT_INDEX;

		// get next free component
		uint32 component_index = s_active_count++;

		// link component to entity
		s_entity_index_data[component_index] = entity_index;

		return component_index;
	}

	TEMPLATE_IMPLEMENTATION(void) Return(uint32 component_index)
	{
		ARC_ASSERT(component_index < s_active_count, "Index out of bounds.");

		s_active_count -= 1;

		// swap components
		s_component_data[component_index] = s_component_data[s_active_count];

		// update swapped component linking
		auto type_id = ManualTypeId<engine::ComponentTypeContext, ComponentT>::Value();
		//entity::_relink_component_to_entity(s_entity_index_data[component_index], type_id, component_index);
		ARC_NOT_IMPLEMENTED();
	}

	TEMPLATE_IMPLEMENTATION(bool) Initialize(memory::Allocator* alloc, uint32 count)
	{
		if (s_entity_index_data != nullptr) return false; // already initialized

		s_maximum_count = count;
		s_active_count = 0;
		s_alloc = alloc;
		s_component_data = (DataT*)alloc->allocate(sizeof(DataT)*count);
		s_entity_index_data = (uint32*)alloc->allocate(sizeof(uint32)*count);

		return true;
	}

	TEMPLATE_IMPLEMENTATION(void) RemovalCallback(Slice<uint32> indices)
	{
		uint32 last_index = -1;

		auto type_id = ManualTypeId<entity::ComponentTypeContext, ComponentT>::Value();

		// reverse iteration
		for (int32 i = (int32)indices.size() - 1; i >= 0; --i)
		{
			uint32 index = indices[i];

			// same index (due to multiple removal), ignore
			if (index == last_index) continue;

			s_active_count -= 1;

			// remove component from entity
			auto orig_entity = s_entity_index_data[index];
			entity::_unlink_component_from_entity(orig_entity, type_id);

			// move last entity into this spot
			s_component_data[index] = s_component_data[s_active_count];
			s_entity_index_data[index] = s_entity_index_data[s_active_count];

			// update swapped component linking
			auto entity_index = s_entity_index_data[index];
			bool ok = entity::_component_index_changed(entity_index, type_id, index);
			ARC_ASSERT(ok, "Entity doesn't have the requested component, something is seriously broken here.")

			// remember last index
			last_index = index;
		}
	}

} // namespace arc

#endif