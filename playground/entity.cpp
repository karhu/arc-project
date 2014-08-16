#include "entity.hpp"

#include "arc/memory/Allocator.hpp"
#include "engine.hpp"
#include "arc/collections/Array.inl"

namespace arc { namespace entity {

		// utility types ////////////////////////////////////////////////////////////

		struct ComponentEntry
		{
			ComponentType type;
			uint32 index;
		};

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

		struct ComponentRemovalCommand
		{
			uint32 index;
			ComponentType type;
		};

		const uint32 INVALID_ENTITY_FREELIST_LINK = std::numeric_limits<uint32>::max();

		// globals //////////////////////////////////////////////////////////////////////////////////

		EntityData*				g_entity_data = nullptr;
		uint32					g_max_entity_count = 0;
		uint32					g_active_entity_count = 0;
		uint32					g_first_free = INVALID_ENTITY_FREELIST_LINK;
		uint32					g_last_free = INVALID_ENTITY_FREELIST_LINK;
		memory::Allocator*		g_entity_alloc = nullptr;

		arc::Array<ComponentRemovalCommand> g_comp_remove_data;

		// helper functions /////////////////////////////////////////////////////////////////////////

		uint32 _get_component(uint32 entity_index, ComponentType type)
		{
			ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds.");

			auto& entity = g_entity_data[entity_index];

			// look for component with matching type
			for (uint32 i = 0; i < entity.active_data.component_count; i++)
			{
				if (entity.active_data.components[i].type == type) return entity.active_data.components[i].index;
			}

			// not found
			return INVALID_COMPONENT_INDEX;
		}

		bool   _has_component(uint32 entity_index, ComponentType type)
		{
			return _get_component(entity_index, type) != INVALID_ENTITY_INDEX;
		}

		uint32 _index(Handle h)
		{
			return h.m_index;
		}

		uint32 _add_component(uint32 entity_index, ComponentType type, uint32 component_index)
		{
			ARC_ASSERT(entity_index <= g_active_entity_count, "Entity index out of bounds");

			auto& entity = g_entity_data[entity_index].active_data;
			ARC_ASSERT(entity.component_count < EntityData::MAX_COMPONENTS, "Maximum number of components for this Entity was reached.");

			entity.components[entity.component_count].type = type;
			entity.components[entity.component_count].index = component_index;

			entity.component_count += 1;
			return true;
		}

		bool   _remove_component_later(Handle h, ComponentType type)
		{
			uint32 entity_index = _index(h);

			// it the entity has the requested component
			if (_has_component(entity_index, type))
			{
				g_comp_remove_data.push_back({ entity_index, type });
				return true;
			}

			// component not present
			return false;
		}

		// function implementations /////////////////////////////////////////////////////////////////

		Handle::Handle(uint32 index) : m_index(index) {}

		bool initialize(lua::State& config)
		{
			if (is_initialized()) return true; // already initialized

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

			// init entity removal data
			g_comp_remove_data.initialize(&engine::longterm_allocator());
			g_comp_remove_data.reserve(128);

			return true;
		}

		bool is_initialized()
		{
			return g_entity_data != nullptr;
		}

		bool finalize()
		{
			g_entity_alloc->free(g_entity_data);
			g_entity_data = nullptr;
			g_max_entity_count = 0;
			g_entity_alloc = nullptr;
			g_first_free = INVALID_ENTITY_FREELIST_LINK;
			g_last_free = INVALID_ENTITY_FREELIST_LINK;

			g_comp_remove_data.finalize();

			return true;
		}

		Handle create()
		{
			ARC_ASSERT(is_initialized(), "entity system is not initialized");

			// if no more enities are available
			// TODO: resize?
			if (g_active_entity_count == g_max_entity_count)
			{
				ARC_ASSERT(false, "Reached the limit of concurrently active entities");
				return{};
			}

			uint32 idx = g_first_free;

			// return first free handle
			Handle h = { idx };

			// pop front of the queue
			g_first_free = g_entity_data[idx].free_data.next;

			// bookkeeping
			g_active_entity_count++;

			// initialize entity data
			auto& ed = g_entity_data[idx].active_data;
			ed.clear();

			return h;
		}

		bool valid(Handle h)
		{
			return _index(h) != INVALID_ENTITY_INDEX;
		}

}} // namespace arc::entity


