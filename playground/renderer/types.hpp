#pragma once

#include "arc/util/IndexPool.hpp"

namespace arc { namespace renderer {

	// ID types ///////////////////////////////////////////////////////////////////////

	DECLARE_ID32(GeometryID);
	DECLARE_ID32(VertexLayoutID);
	DECLARE_ID32(ShaderID);
	DECLARE_ID32(RenderPassID);
	DECLARE_ID32(GeometryConfigID);

	static const GeometryID       INVALID_GEOMETRY_ID = GeometryID(0);
	static const VertexLayoutID   INVALID_VERTEX_LAYOUT_ID = VertexLayoutID(0);
	static const ShaderID         INVALID_SHADER_ID = ShaderID(0);
	static const RenderPassID     INVALID_RENDER_PASS_ID = RenderPassID(0);
	static const GeometryConfigID INVALID_GEOMETRY_CONFIG_ID = GeometryConfigID(0);

	// Untyped Buffer /////////////////////////////////////////////////////////////////

	struct UntypedBuffer
	{
		void*  ptr;
		size_t size;

		inline bool valid() { return ptr != nullptr; }
		inline bool valid(size_t min_size) { return valid() && size >= min_size; }

		template <typename T> inline
		T& access(size_t byte_offset)
		{
			ARC_ASSERT(byte_offset + sizeof(T) <= size, "out of bounds access");
			return *static_cast<T*>(memory::util::ptr_add(ptr, byte_offset));
		}
	};

	static const UntypedBuffer INVALID_UNTYPED_BUFFER = UntypedBuffer{ nullptr, 0 };

	// IndexType //////////////////////////////////////////////////////////////////////

	enum class IndexType : uint8
	{
		None = 0,
		Uint8 = 1,
		Uint16 = 2,
		Uint32 = 4,
	};

	// PrimitiveType //////////////////////////////////////////////////////////////////////

	enum class PrimitiveType : uint8
	{
		Unknown = 0,
		Triangle,
	};

	// GeometryBufferType /////////////////////////////////////////////////////////////////

	enum class GeometryBufferType : uint8
	{
		Static = 0,
		COUNT
	};

	// PrimitiveTypes /////////////////////////////////////////////////////////////////////

	enum class ShaderPrimitiveType : uint8
	{
		none_t,
		float_t,
		vec2_t,
		vec3_t,
		vec4_t,
		mat2x2_t,
		mat3x3_t,
		mat4x4_t,
		uint_t,
		uvec2_t,
		uvec3_t,
		uvec4_t,
	};

	enum class ShaderUniformType : uint8
	{
		Undefined = 0,
		Instanced = 1,
		Material = 2,
		Global = 3,
	};

}}