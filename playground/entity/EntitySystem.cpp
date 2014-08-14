#if 0

#include "EntitySystem.hpp"

#include "arc/memory/allocator.hpp"

namespace arc { namespace engine {

	// EntityHandle /////////////////////////////////////////////////////////////////////////////////////

	EntityHandle::EntityHandle(uint32 index) : m_index(index) {}
	bool EntityHandle::valid() { return m_index != INVALID_ENTITY_INDEX; }

	// EntitySystem globals //////////////////////////////////////////////////////////////////////////////

	struct ComponentEntry
	{
		EntitySystem::ComponentType type;
		uint32 index;
	};

	struct LinkedComponentEntry
	{
		EntitySystem::ComponentType type;
		uint32 index;
		uint32 next;
	};

	static const uint32 INVALID_ENTITY_FREELIST_LINK = -1;

	union EntityData
	{
		static const uint32 MAX_COMPONENTS = 16;

		struct 
		{
			uint8 component_count;
			ComponentEntry components[MAX_COMPONENTS];

			void clear() { component_count = 0; }
		} active_data;

		struct
		{
			uint32 next;
		} free_data;
	};

	EntityData*				g_entity_data = nullptr;
	uint32					g_max_entity_count = 0;
	uint32					g_active_entity_count = 0;
	uint32					g_first_free = INVALID_ENTITY_FREELIST_LINK;
	uint32					g_last_free = INVALID_ENTITY_FREELIST_LINK;
	memory::Allocator*		g_entity_alloc = nullptr;

	// EntitySystem /////////////////////////////////////////////////////////////////////////////////////

	ARC_SUBSYSTEM_DEFINITION(arc::engine::EntitySystem);

	EntitySystem::EntitySystem(memory::Allocator& alloc) {}

	bool EntitySystem::initialize(lua::State& config)
	{
		if (g_entity_data != nullptr) return false; // already initialized

		auto alloc = &engine::longterm_allocator();

		// TODO read config
		uint32 max_entities = 512;

		// init entity memory
		g_entity_alloc = alloc;
		g_entity_data = (EntityData*)alloc->allocate(sizeof(EntityData)*max_entities, alignof(EntityData));
		g_max_entity_count = max_entities;

		// initialize free list
		for (uint32 i = 0; i < max_entities; i++)
		{
			g_entity_data[i].free_data.next = i + 1;
		}
		g_entity_data[max_entities - 1].free_data.next = INVALID_ENTITY_FREELIST_LINK;
		g_first_free = 0;

		return true;
	}

	bool EntitySystem::finalize()
	{
		g_entity_alloc->free(g_entity_data);
		g_entity_data = nullptr;
		g_max_entity_count = 0;
		g_entity_alloc = nullptr;
		g_first_free = INVALID_ENTITY_FREELIST_LINK;
		g_last_free = INVALID_ENTITY_FREELIST_LINK;

		return true;
	}

	EntityHandle EntitySystem::create_entity()
	{
		// if no more enities are available
		// TODO: resize?
		if (g_active_entity_count == g_max_entity_count)
		{
			ARC_ASSERT(false, "Reached the limit of concurrently active entities");
			return {};
		}

		uint32 idx = g_first_free;

		// return first free handle
		EntityHandle h = { idx };

		// pop front of the queue
		g_first_free = g_entity_data[idx].free_data.next;

		// bookkeeping
		g_active_entity_count++;

		// initialize entity data
		auto& ed = g_entity_data[idx].active_data;
		ed.clear();

		return h;
	}

	uint32 EntitySystem::available_entity_count() { return g_max_entity_count - g_active_entity_count; }
	uint32 EntitySystem::active_entity_count() { return g_active_entity_count; }

	// implementation functions ///////////////////////////////////////////////////////////////////////////////

	/* updates the EXISTING component index stored in an entity */
	bool _relink_component_to_entity(uint32 entity_index, EntitySystem::ComponentType component_type, uint32 component_index)
	{
		ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds");

		auto& entity = g_entity_data[entity_index].active_data;

		// look for component with matching type
		for (uint32 i = 0; i < entity.component_count; i++)
		{
			if (entity.components[i].type == component_type)
			{
				// successfull relinking
				entity.components[i].index = component_index;
				return true;
			}
		}

		return false;
	}

	/* adds the component to the entity, assuming no component of the given type was present so far */
	bool _link_component_to_entity(uint32 entity_index, EntitySystem::ComponentType component_type, uint32 component_index)
	{
		ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds");
		ARC_ASSERT(!_entity_has_component(entity_index, component_type), "Component of given type is already present");

		auto& entity = g_entity_data[entity_index];
		ARC_ASSERT(entity.active_data.component_count < EntityData::MAX_COMPONENTS, "Maximum number of components for this Entity was reached");

		entity.active_data.components[entity.active_data.component_count].type = component_type;
		entity.active_data.components[entity.active_data.component_count].index = component_index;

		entity.active_data.component_count += 1;
		return true;
	}

	/* removes a single component from the entity */
	uint32 _unlink_component_from_entity(uint32 entity_index, EntitySystem::ComponentType component_type)
	{
		ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds");

		auto& entity = g_entity_data[entity_index];

		// look for component with matching type
		for (uint32 i = 0; i < entity.active_data.component_count; i++)
		{
			if (entity.active_data.components[i].type == component_type)
			{
				auto removed_index = entity.active_data.components[i].index;
				entity.active_data.component_count -= 1;
				entity.active_data.components[i] = entity.active_data.components[entity.active_data.component_count];
				return removed_index;
			}
		}

		// not found
		return INVALID_COMPONENT_INDEX;
	}

	bool _entity_has_component(uint32 entity_index, EntitySystem::ComponentType component_type)
	{
		return _entity_get_component(entity_index, component_type) != INVALID_COMPONENT_INDEX;
	}

	uint32 _entity_get_component(uint32 entity_index, EntitySystem::ComponentType component_type)
	{
		ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds.");

		auto& entity = g_entity_data[entity_index];

		// look for component with matching type
		for (uint32 i = 0; i < entity.active_data.component_count; i++)
		{
			if (entity.active_data.components[i].type == component_type) return entity.active_data.components[i].index;
		}

		// not found
		return INVALID_COMPONENT_INDEX;
	}

	bool _entity_destroy_entity(uint32 entity_index)
	{
		ARC_ASSERT(entity_index < g_max_entity_count, "Entity index out of bounds.");

		// filter invalid (or out of bounds) indices
		if (entity_index >= g_max_entity_count) return false;

		// in case of empty entity reserves
		if (g_active_entity_count == g_max_entity_count)
		{
			g_first_free = g_last_free = entity_index;
		}
		// usual case 
		else
		{
			// push onto end of linked list
			g_entity_data[g_last_free].free_data.next = entity_index;
			g_last_free = entity_index;
		}
		
		// properly end the linked list
		g_entity_data[entity_index].free_data.next = INVALID_ENTITY_FREELIST_LINK;
		return true;
	}

}} // namespace arc::engine

#endif