#pragma once

#include "arc/hash/StringHash.hpp"
#include "arc/math/vectors.hpp"
#include "arc/memory/Allocator.hpp"

#include <array>

namespace arc
{

	struct DynamicTypeData
	{
		const char* name;
		const uint16 alignment;
		const uint16 size;
	};

	struct Field
	{
		Field(const DynamicTypeData* type, StringHash32 name_hash, const char* name)
			: m_type(type), m_name_hash(name_hash), m_name(name)
		{}

		const DynamicTypeData*  m_type;
		arc::StringHash32 m_name_hash;
		const char* m_name;
	};

	template<typename T>
	struct TypeInfo
	{};

	template<>
	struct TypeInfo < float >
	{
		static const DynamicTypeData* dynamic;
	};

	template<>
	struct TypeInfo < vec3 >
	{
		static const DynamicTypeData* dynamic;
	};

	template<typename T, uint32 VALUE>
	struct Index
	{
		static const uint32 Value = VALUE;
		using Type = T;
	};

	struct Description
	{
		static const uint32 field_count = 2;

		static const std::array<Field, field_count> fields;

		using SCALE = Index<float,0>;
		using POSITION = Index<vec3,1>;
	};

	template<typename D>
	struct SOA
	{
		SOA(memory::Allocator* alloc, uint32 size)
			: m_size(size), m_alloc(alloc)
		{
			for (uint32 i = 0; i < D::fields.size(); i++)
			{
				auto& type = *D::fields[i].m_type;
				m_data[i] = m_alloc->allocate(m_size*type.size, type.alignment);
			}
		}

		~SOA()
		{
			for (uint32 i = 0; i < D::fields.size(); i++)
			{
				m_alloc->free(m_data[i]);
			}
		}

		template<typename T>
		T& get(uint32 index)
		{
			ARC_ASSERT(T::Value <= D::fields.size(), "field index out of bounds");
			ARC_ASSERT(D::fields[T::Value].m_type == TypeInfo<T::Type>::dynamic, "wrong type");
			ARC_ASSERT(index <= m_size, "array index out of bouds");

			auto a = static_cast<T*>(m_data[T::Value]);
			return a[index];
		}

		void* m_data[D::field_count];
		uint32 m_size = 0;
		memory::Allocator* m_alloc;
	};



}