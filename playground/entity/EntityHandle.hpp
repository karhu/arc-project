#pragma once

#include "common.hpp"

namespace arc { namespace engine { 

	class EntityHandle
	{
	public: // constructor
		EntityHandle(uint32 index = INVALID_ENTITY_INDEX);

	public: // validity check
		bool valid();

	public: // state interface
		bool destroy();

	public: // component interface
		template<typename T>
		bool remove();

		template<typename T>
		typename T add();

		template<typename T>
		typename T get();

	private:
		uint32 m_index;
	};

	// implementation ///////////////////////////////////////////

	template<typename T>
	bool EntityHandle::remove()
	{
		ARC_ASSERT(valid(), "Trying to use invalid EntityInterface");
		auto type_id = ManualTypeId<ComponentIdContext, T>::Value();
		uint32 component_index = entities::unlink_component_from_entity(m_index, type_id);
		if (component_index == INVALID_COMPONENT_INDEX) return false; // component was not present

		T::Return(component_index);
		return true;
	};

	template<typename T>
	typename T EntityHandle::add()
	{
		ARC_ASSERT(valid(), "Trying to use invalid EntityInterface");

		auto type_id = ManualTypeId<ComponentIdContext, T>::Value();

		// check if the component is already present
		if (_entity_has_component(m_index, type_id)) return{};

		uint32 component_index = T::Create(m_index);
		bool ok = _link_component_to_entity(m_index, type_id, component_index);

		if (!ok) T::Return(component_index);

		return T::Get(component_index);
	}

	template<typename T>
	typename T EntityHandle::get()
	{
		ARC_ASSERT(valid(), "Trying to use invalid EntityInterface");

		auto type_id = ManualTypeId<ComponentIdContext, T>::Value();
		auto component_index = _entity_get_component(m_index, type_id);
		return T::Get(component_index);
	}

	inline bool EntityHandle::destroy()
	{
		ARC_ASSERT(valid(), "Trying to use invalid EntityInterface");
		ARC_NOT_IMPLEMENTED;

		//return _entity_destroy_entity(m_index);
	}

}} // namespace arc::engine