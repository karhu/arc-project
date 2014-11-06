#pragma once

#include "arc/core.hpp"
#include "arc/lua/State.hpp"
#include "arc/util/ManualTypeId.hpp"
#include "arc/collections/Slice.hpp"

// public interface ///////////////////////////////////////////
namespace arc { namespace entity {

		using ComponentIndex = uint32;
		using ComponentType  = uint8;

		const uint32 INVALID_ENTITY_INDEX = std::numeric_limits<uint32>::max();
		const uint32 INVALID_COMPONENT_INDEX = std::numeric_limits<uint32>::max();

		struct Handle
		{
		public: // constructor
			Handle(uint32 index = INVALID_ENTITY_INDEX);
		private:
			uint32 m_index;
		private:
			friend uint32 _index(Handle h);
		};

		bool initialize(lua::State& config);
		bool finalize();
		bool is_initialized();

		template<typename T>
		bool register_component(uint32 max_count);

		Handle create();
		//bool destroy(Handle h);

		bool valid(Handle h);
		
		template<typename T>
		T get(Handle h);

		template<typename T>
		bool has(Handle h);

		template<typename T>
		T add(Handle h);

		template<typename T>
		bool remove(Handle h);

		// update functions

		void update_component_removal();

}} // namespace arc::entity

// callback functionality /////////////////////////////////////////

namespace arc { namespace entity {

	using CallbackFunction = void(*)();

	enum class CallbackType : uint16
	{
		Update = 0,
		RenderPre,
		RenderMain,
		RenderPost,
		COUNT,
	};

	void register_callback(CallbackType type, float priority, CallbackFunction function);
	void call_callbacks(CallbackType type);

}}

// implementation utils ///////////////////////////////////////////
namespace arc { namespace entity {

	using _ComponentRemovalCallback = void(*)(Slice<uint32> component_indices);

	uint32 _index(Handle h);
	uint32 _get_component(uint32 entity_index, ComponentType type);
	bool   _has_component(uint32 entity_index, ComponentType type);
	uint32 _add_component(uint32 entity_index, ComponentType type, uint32 component_index);
	bool   _remove_component_later(Handle h, ComponentType type);
	bool   _register_component_removal_callback(ComponentType type, _ComponentRemovalCallback rm_cb);
	bool   _component_index_changed(uint32 entity_index, ComponentType type, uint32 new_component_index);
	bool   _unlink_component_from_entity(uint32 entity_index, ComponentType type);

	struct ComponentTypeContext
	{
		using Type = uint8;
		static const uint8 First = 0;
		static const uint8 Last = 254;
		static const uint8 Invalid = 255;
	};

}} // namespace arc::entity

// template function implementations //////////////////////////////
namespace arc { namespace entity {

	template<typename T>
	bool register_component(uint32 max_count)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");

		auto& alloc = engine::longterm_allocator();
		bool ok = T::Backend::Initialize(&alloc, max_count);

		if (ok)
		{
			ManualTypeId<ComponentTypeContext, T>::Initialize();
			auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();
			_register_component_removal_callback(type_id, &T::Backend::RemovalCallback);

			T::Backend::RegisterCallbacks();
			return true;
		}

		return false;
	}

	template<typename T>
	T get(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityInterface");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();
		auto component_index = _get_component(_index(h), type_id);
		return T::Backend::Get(component_index);
	}

	template<typename T>
	bool has(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");
		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();
		return _has_component(_index(h), type_id);
	}

	template<typename T>
	T add(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();

		// check if the component is already present
		if (_has_component(_index(h), type_id)) return {};

		// create the component
		uint32 component_index = T::Backend::Create(_index(h));
		_add_component(_index(h), type_id, component_index);

		return T::Backend::Get(component_index);
	}

	template<typename T>
	bool remove(Handle h)
	{
		ARC_ASSERT(is_initialized(), "Entity system is not initialized.");
		ARC_ASSERT(valid(h), "Trying to use invalid EntityHandle.");

		auto type_id = ManualTypeId<ComponentTypeContext, T>::Value();

		return _remove_component_later(h, type_id);
	}

}} // namespace arc::entity


namespace arc {

	/*
	template<typename ...Types>
	class StructOfArrays
	{
	public:
	static const uint32 FIELD_COUNT = sizeof...(Types);
	public:
	void initialize(memory::Allocator* alloc, uint32 size)
	{
	ARC_ASSERT(m_alloc == nullptr, "Already initialized.");

	m_size = size;
	m_alloc = alloc;

	parameter_pack<Types...>::foreach<InitIterator>(*this);
	}

	public:
	template<uint32 FieldIndex, typename T>
	T& get(uint32 index)
	{
	static_assert(FieldIndex < FIELD_COUNT, "Field index out of bounds.");
	using FieldType = pack_element<FieldIndex, Types...>::type;
	static_assert(std::is_same<T, FieldType>::value, "Incorrect type requested");

	ARC_ASSERT(index < m_size, "Invalid element index: %u",index);

	FieldType* data = static_cast<FieldType*>(m_data[FieldIndex]);
	return data[index];
	}

	private:
	using ThisT = StructOfArrays < Types... > ;

	struct InitIterator { template<typename T> static bool iteration(uint32 index, ThisT& self)
	{
	self.m_data[index] = self.m_alloc->allocate(sizeof(T)*self.m_size, alignof(T));
	return true;
	}};

	private:
	void* m_data[FIELD_COUNT];
	uint32 m_size = 0;
	memory::Allocator* m_alloc = nullptr;
	};

	*/

}