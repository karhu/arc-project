#pragma once

#include <algorithm>

#include "arc/common.hpp"
#include "arc/hash/StringHash.hpp"
#include "arc/gl/types.hpp"
#include "arc/memory/Allocator.hpp"

#include "types.hpp"

namespace arc { namespace renderer {

	struct VertexAttribute
	{
	public:
		enum class Type : uint8
		{
			undefined = 0,
			float16, float32, float64,
			uint8, uint16, uint32,
			int8, int16, int32,
			uint8_nf, uint16_nf, uint32_nf,
			int8_nf, int16_nf, int32_nf,
		};
	public:
		inline StringHash32 hash() const { return m_hash; }
		inline Type type() const       { return m_type; }
		inline uint8 elements() const  { return m_elements; }
		inline uint8 offset() const    { return m_offset; }
	public:
		inline VertexAttribute(StringHash32 hash, Type type, uint8 elements, uint8 offset)
			: m_hash(hash), m_type(type), m_elements(elements), m_offset(offset) {}

		inline VertexAttribute()
			: m_hash(0), m_type(Type::undefined), m_elements(0), m_offset(0) {}

		inline VertexAttribute(const VertexAttribute& other) = default;
	public:
		inline bool operator<(const VertexAttribute& rhs) const
		{
			auto v = m_hash.value();
			auto rhsv = rhs.m_hash.value();
			return std::tie(v, m_type, m_elements, m_offset) < std::tie(rhsv, rhs.m_type, rhs.m_elements, rhs.m_offset);
		}

		inline bool operator==(const VertexAttribute& rhs) const
		{
			auto v = m_hash.value();
			auto rhsv = rhs.m_hash.value();
			return std::tie(v, m_type, m_elements, m_offset) == std::tie(rhsv, rhs.m_type, rhs.m_elements, rhs.m_offset);
		}

		inline bool operator!=(const VertexAttribute& rhs) const
		{
			auto v = m_hash.value();
			auto rhsv = rhs.m_hash.value();
			return std::tie(v, m_type, m_elements, m_offset) != std::tie(rhsv, rhs.m_type, rhs.m_elements, rhs.m_offset);
		}
	private:
		StringHash32 m_hash;
		Type m_type;
		uint8 m_elements;
		uint8 m_offset;
	};

	struct VertexLayout
	{
	public:
		inline StringHash32 name_hash() const                    { return m_name_hash; }
		inline uint8 count() const                               { return m_count; }
		inline const VertexAttribute& attribute(uint8 idx) const { return m_attributes[idx]; }
		inline uint8 stride() const                              { return m_stride; }
		inline const Slice<VertexAttribute> attributes() const   { return make_slice(m_attributes, m_count); }
	public:
		const VertexAttribute* find_attribute(StringHash32 name_hash) const;
	public:
		inline VertexLayout(StringHash32 name_hash, VertexAttribute* attributes, uint8 count, uint8 stride)
			: m_name_hash(name_hash), m_attributes(attributes), m_count(count), m_stride(stride) 
		{
			std::sort(m_attributes, m_attributes + m_count);
		}
		inline VertexLayout(StringHash32 name_hash, uint8 stride, const Slice<VertexAttribute> attributes, memory::Allocator* alloc)
			: m_name_hash(name_hash), m_count(static_cast<uint8_t>(attributes.size())), m_stride(stride), m_alloc(alloc)
		{
			m_attributes = alloc->create_n<VertexAttribute>(m_count);
			for (uint32_t i = 0; i < m_count; i++) m_attributes[i] = attributes[i];
			std::sort(m_attributes, m_attributes + m_count);
		}
		inline ~VertexLayout()
		{
			if (m_alloc) m_alloc->destroy_n(m_attributes, m_count);
		}
	private:
		ARC_NO_COPY(VertexLayout);
	private:
		StringHash32 m_name_hash;
		VertexAttribute* m_attributes = nullptr;
		uint8 m_count = 0;
		uint8 m_stride = 0;
		memory::Allocator* m_alloc = nullptr;
	private:
		friend bool equal(const VertexLayout& a, const VertexLayout& b);
	};

	VertexLayout* copy(const VertexLayout& src, memory::Allocator& alloc);

	bool equal(const VertexLayout& a, const VertexLayout& b);

	inline const VertexAttribute* VertexLayout::find_attribute(StringHash32 name_hash) const
	{
		for (uint32 i = 0; i < m_count; i++)
		{
			if (m_attributes[i].hash() == name_hash) return m_attributes + i;
		}
		return nullptr;
	}

	gl::IndexType type_gl(IndexType t);
	uint32 type_gl(VertexAttribute::Type t);
	bool type_normalize(VertexAttribute::Type t);
	inline uint32 byte_size(IndexType t) { return (uint32)t; }

	// DEPRECATED
	struct MeshDataOld
	{
		uint32 vertex_count;
		uint32 index_count;

		uint32 vertex_offset; // vertex offset in vertices
		uint32 index_offset;  // index offset in index values

		uint32 buffer;
		IndexType index_type = IndexType::None;
		VertexLayout* vertex_layout = nullptr;
	};

}}