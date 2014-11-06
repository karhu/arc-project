#pragma once

#include "arc/common.hpp"
#include "arc/util/IndexPool.hpp"
#include "arc/util/ManualTypeId.hpp"
#include "arc/collections/Array.hpp"

namespace arc { namespace entity2 {

	class Context;

	struct Handle
	{
	public:
		inline uint32 index() { return m_index; }
	private:
		uint32 m_index : 24;
		uint32 m_generation : 8;
	private:
		friend class entity2::Context;
	};

	class ComponentBase
	{

	};

	class Context
	{
	public:
		Context(memory::Allocator* alloc, uint32 initial_capacity);
		~Context();
	public:
		template<typename T>
		bool register_component(uint32 initial_capacity);
		template<typename T>
		typename T::Backend& get_backend();
	public:
		entity2::Handle create_entity();
		bool destroy_entity(entity2::Handle h);
	public:
		bool valid(entity2::Handle h);
	public:
		template<typename T>
		typename T::Handle add_component(entity2::Handle h);

		template<typename T>
		typename T::Handle get_component(entity2::Handle h);

		template<typename T>
		bool has_component(entity2::Handle h);

		template<typename T>
		bool remove_component(entity2::Handle h);
	private:
		struct CompTContext
		{
			using Type = uint8;
			static const uint8 First = 0;
			static const uint8 Last = 254;
			static const uint8 Invalid = 255;
		};
		struct ComponentEntry
		{
			CompTContext::Type m_type;
			uint32 m_index;
		};
		struct EntityData
		{
			uint16 m_component_count = 0;
		};

	private:
		uint32 m_index;
		IndexPool32 m_index_pool;
		memory::Allocator* m_allocator = nullptr;

		ComponentBase* m_component_backends[CompTContext::Last + 1];
	};


	// template implementations ///////////////////////////////////////////////////////////

	template<typename T>
	bool Context::register_component(uint32 initial_capacity)
	{
		ManualTypeId<CompTContext, T>::AssertInitialized();
		auto type_id = ManualTypeId<CompTContext, T>::Value();
		
		if (m_component_backends[type_id])
		{
			ARC_ASSERT(false, "type is already registered");
			return false;
		}

		m_component_backends[type_id] = m_allocator->create<T>(m_allocator, initial_capacity);
		return true;
	}

	template<typename T>
	typename T::Handle Context::add_component(entity2::Handle h)
	{
		ARC_ASSERT(valid(h), "invalid entity handle");

		auto type_id = ManualTypeId<CompTContext, T>::Value();
		ARC_ASSERT(type_id != CompTContext::Invalid, "Invalid Component Type");

		auto backend = m_component_backends[type_id];
		ARC_ASSERT(backend != nullptr, "Unregistered Component Type");

		auto typed_backend = static_cast<T*>(backend);

		return typed_backend->add_component(h);
	}

	template<typename T>
	typename T::Handle Context::get_component(entity2::Handle h)
	{
		ARC_ASSERT(valid(h), "invalid entity handle");

		auto type_id = ManualTypeId<CompTContext, T>::Value();
		ARC_ASSERT(type_id != CompTContext::Invalid, "Invalid Component Type");

		auto backend = m_component_backends[type_id];
		ARC_ASSERT(backend != nullptr, "Unregistered Component Type");

		auto typed_backend = static_cast<T*>(backend);

		return typed_backend->get_component(h);
	}
#if 0 
	template<typename T>
	typename T::Backend& Context::get_backend()
	{
		ARC_ASSERT(valid(h), "invalid entity handle");

		auto type_id = ManualTypeId<CompTContext, T>::Value();
		ARC_ASSERT(type_id != CompTContext::Invalid, "Invalid Component Type");

		auto backend = m_component_backends[type_id];
		ARC_ASSERT(backend != nullptr, "Unregistered Component Type");

		auto typed_backend = reinterpret_cast<T::Backend*>(backend);

		return *typed_backend;
	}

	template<typename T>
	bool Context::remove_component(entity2::Handle h)
	{
		ARC_ASSERT(valid(h), "invalid entity handle");
		ARC_NOT_IMPLEMENTED;
		//return T::Backend::Remove_Component(*this, h.m_index, -1);
	}
#endif
	template<typename T>
	bool Context::has_component(entity2::Handle h)
	{
		ARC_ASSERT(valid(h), "invalid entity handle");

		auto type_id = ManualTypeId<CompTContext, T>::Value();
		ARC_ASSERT(type_id != CompTContext::Invalid, "Invalid Component Type");

		auto backend = m_component_backends[type_id];
		ARC_ASSERT(backend != nullptr, "Unregistered Component Type");

		auto typed_backend = static_cast<T*>(backend);

		return typed_backend->get_component(h).valid();
	}


}}