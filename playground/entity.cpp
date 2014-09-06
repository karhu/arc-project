#include "entity.hpp"

#include "engine.hpp"

#include "arc/memory/Allocator.hpp"
#include "arc/collections/Array.inl"
#include "arc/util/ZipIterator.hpp"

#include <algorithm>

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

		arc::Array<uint32> g_comp_remove_index_data;
		arc::Array<uint32> g_comp_remove_type_data;
		
		arc::Array<_ComponentRemovalCallback> g_comp_removal_callbacks;

		arc::Array<CallbackFunction> g_callbacks[(uint32)CallbackType::COUNT];

		// helper functions /////////////////////////////////////////////////////////////////////////

		uint32 _get_component(uint32 entity_index, ComponentType type)
		{
			ARC_ASSERT(entity_index <= g_max_entity_count, "Entity index out of bounds.");

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
			ARC_ASSERT(entity_index <= g_max_entity_count, "Entity index out of bounds");

			auto& entity = g_entity_data[entity_index].active_data;
			ARC_ASSERT(entity.component_count < EntityData::MAX_COMPONENTS, "Maximum number of components for this Entity was reached.");

			entity.components[entity.component_count].type = type;
			entity.components[entity.component_count].index = component_index;

			entity.component_count += 1;
			return true;
		}

		bool _remove_component_later(Handle h, ComponentType type)
		{
			uint32 entity_index = _index(h);

			// it the entity has the requested component
			if (_has_component(entity_index, type))
			{
				g_comp_remove_index_data.push_back(entity_index);
				g_comp_remove_type_data.push_back(type);
				return true;
			}

			// component not present
			return false;
		}

		bool _register_component_removal_callback(ComponentType type, _ComponentRemovalCallback rm_cb)
		{
			auto& callbacks = g_comp_removal_callbacks;
			uint32 type_index = type;

			// make sure we have the necessary storage
			if (callbacks.size() <= type_index) callbacks.resize(type_index + 1, nullptr);

			// no callback registered for this type
			if (callbacks[type_index] == nullptr)
			{
				callbacks[type_index] = rm_cb;
				return true;
			}
			// there was already a callback registered for this type
			return false;
		}

		bool _component_index_changed(uint32 entity_index, ComponentType type, uint32 new_component_index)
		{
			ARC_ASSERT(entity_index <= g_max_entity_count, "Entity index out of bounds.");

			auto& entity = g_entity_data[entity_index];

			// look for component with matching type
			for (uint32 i = 0; i < entity.active_data.component_count; i++)
			{
				if (entity.active_data.components[i].type == type)
				{
					entity.active_data.components[i].index = new_component_index;
					return true;
				}
			}

			// not found
			return false;
		}

		bool _unlink_component_from_entity(uint32 entity_index, ComponentType type)
		{
			ARC_ASSERT(entity_index <= g_max_entity_count, "Entity index out of bounds.");

			auto& entity = g_entity_data[entity_index];

			// look for component with matching type
			for (uint32 i = 0; i < entity.active_data.component_count; i++)
			{
				if (entity.active_data.components[i].type == type)
				{
					// move last entry into this ones spot
					entity.active_data.components[i] = entity.active_data.components[entity.active_data.component_count - 1];
					entity.active_data.component_count -= 1;
					return true;
				}
			}

			// not found
			return false;
		}

		// function implementations /////////////////////////////////////////////////////////////////

		Handle::Handle(uint32 index) : m_index(index) {}

		bool initialize(lua::State& config)
		{
			// check and handle already initialized
			if (is_initialized()) return true; 

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
			g_comp_remove_index_data.initialize(&engine::longterm_allocator());
			g_comp_remove_index_data.reserve(128);

			g_comp_remove_type_data.initialize(&engine::longterm_allocator());
			g_comp_remove_type_data.reserve(128);

			g_comp_removal_callbacks.initialize(&engine::longterm_allocator());
			g_comp_removal_callbacks.reserve(32);

			// initialize other callbacks
			for (uint32 i = 0; i < (uint32)CallbackType::COUNT; i++)
			{
				g_callbacks[i].initialize(&engine::longterm_allocator());
				g_callbacks[i].reserve(32);
			}

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

			g_comp_remove_index_data.finalize();
			g_comp_remove_type_data.finalize();
			g_comp_removal_callbacks.finalize();

			// finalize other callbacks
			for (uint32 i = 0; i < (uint32)CallbackType::COUNT; i++)
			{
				g_callbacks[i].finalize();
			}

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

		void update_component_removal()
		{
			ARC_ASSERT(is_initialized(), "entity system is not initialized");

			// early out
			if (g_comp_remove_index_data.size() == 0) return;

			auto itr_begin = make_zip_iterator(begin(g_comp_remove_type_data), begin(g_comp_remove_index_data));
			auto itr_end = make_zip_iterator(end(g_comp_remove_type_data), end(g_comp_remove_index_data));

			// create continuous blocks of same type and increasing index
			std::sort(itr_begin, itr_end);

			// handle each continuous slice separately
			ComponentType current_type = g_comp_remove_type_data[0];
			uint32 first_index = 0;

			for (uint32 i = 1; i < g_comp_remove_index_data.size(); i++)
			{
				auto type = g_comp_remove_type_data[i];
				if (type != current_type)
				{
					// lookup correct removal callback function for current_type
					auto cb = g_comp_removal_callbacks[current_type];
					ARC_ASSERT(cb, "No removal callback is registered for component type %d.", current_type);

					// call callback function
					auto indices = make_slice(g_comp_remove_index_data.data() + first_index, i - first_index);

					cb(indices);
					// continue
					first_index = i;
					current_type = type;
				}
			}

			// handle last slice of the array

			// lookup correct removal callback function for current_type
			auto cb = g_comp_removal_callbacks[current_type];
			ARC_ASSERT(cb, "No removal callback is registered for component type %d.", current_type);
			
			// call callback function
			auto indices = make_slice(g_comp_remove_index_data.data() + first_index, g_comp_remove_index_data.size() - first_index);
			cb(indices);

			// clear temporary data
			g_comp_remove_index_data.clear();
			g_comp_remove_type_data.clear();
		}

		void register_callback(CallbackType type, float priority, CallbackFunction function)
		{
			ARC_ASSERT(is_initialized(), "entity system is not initialized");

			g_callbacks[(uint32)type].push_back(function);
		}

		void call_callbacks(CallbackType type)
		{
			ARC_ASSERT(is_initialized(), "entity system is not initialized");

			for (uint32 i = 0; i < (uint32)CallbackType::COUNT; i++)
			{
				for (auto& cb : g_callbacks[i])
				{
					cb();
				}
			}
		}

}} // namespace arc::entity


