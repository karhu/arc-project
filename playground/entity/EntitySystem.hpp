#if 0
#pragma once

#include "../engine.hpp"
#include "common.hpp"

#include "EntityHandle.hpp"

namespace arc { namespace engine {

	class EntitySystem : public Subsystem
	{
		ARC_SUBSYSTEM_DECLARATION(EntitySystem);
	public:
		using ComponentType = uint8;
	public:
		template<typename T>
		bool register_component(memory::Allocator* alloc, uint32 max_count);
	public: 
		EntityHandle create_entity();
	public: // statistics
		uint32 available_entity_count();
		uint32 active_entity_count();
	};

	// implementation functions /////////////////////////////////////////////////////////////////////////////

	bool _relink_component_to_entity(uint32 entity_index, EntitySystem::ComponentType component_type, uint32 component_index);
	bool _link_component_to_entity(uint32 entity_index, EntitySystem::ComponentType component_type, uint32 component_index);
	uint32 _unlink_component_from_entity(uint32 entity_index, EntitySystem::ComponentType component_type);

	bool   _entity_has_component(uint32 entity_index, EntitySystem::ComponentType component_type);
	uint32 _entity_get_component(uint32 entity_index, EntitySystem::ComponentType component_type);
	bool   _entity_destroy_entity(uint32 entity_index);

	// implementation ///////////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	bool EntitySystem::register_component(memory::Allocator* alloc, uint32 max_count)
	{
		bool ok = T::Initialize(alloc, max_count);

		if (ok)
		{
			ManualTypeId<ComponentIdContext, T>::Initialize();
			return true;
		}

		return false;
	}

}} // namespace arc::engine

#endif