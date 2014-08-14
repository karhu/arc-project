#pragma once

#include "arc/core.hpp"
#include "arc/math/vectors.hpp"
#include "arc/util/ManualTypeId.hpp"

using namespace arc;

// TODO:
// - second ComponentType
// - deferred removal
// - free list for entities
// - register components

#include "template_util.hpp"

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