
#include "VertexLayout.hpp"

namespace arc { namespace renderer {

	uint32 type_gl(VertexAttribute::Type t)
	{
		using T = VertexAttribute::Type;
		switch (t)
		{
		case T::float16:   return GL_HALF_FLOAT;
		case T::float32:   return GL_FLOAT;
		case T::float64:   return GL_DOUBLE;
		case T::uint8:     return GL_UNSIGNED_BYTE;
		case T::uint16:    return GL_UNSIGNED_SHORT;
		case T::uint32:    return GL_UNSIGNED_INT;
		case T::int8:      return GL_BYTE;
		case T::int16:     return GL_SHORT;
		case T::int32:     return GL_INT;
		case T::uint8_nf:  return GL_UNSIGNED_BYTE;
		case T::uint16_nf: return GL_UNSIGNED_SHORT;
		case T::uint32_nf: return GL_UNSIGNED_INT;
		case T::int8_nf:   return GL_BYTE;
		case T::int16_nf:  return GL_SHORT;
		case T::int32_nf:  return GL_INT;
		};

		ARC_ASSERT(false, "undhandled type");
		return 0;
	}

	bool type_normalize(VertexAttribute::Type t)
	{
		using T = VertexAttribute::Type;
		switch (t)
		{
		case T::float16:
		case T::float32:
		case T::float64:
		case T::uint8:
		case T::uint16:
		case T::uint32:
		case T::int8:
		case T::int16:
		case T::int32:
			return false;
		case T::uint8_nf:
		case T::uint16_nf:
		case T::uint32_nf:
		case T::int8_nf:
		case T::int16_nf:
		case T::int32_nf:
			return true;
		}

		ARC_ASSERT(false, "undhandled type");
		return true;
	}

	gl::IndexType type_gl(IndexType t)
	{
		switch (t)
		{
		case IndexType::Uint8:  return gl::IndexType::u8;
		case IndexType::Uint16: return gl::IndexType::u16;
		case IndexType::Uint32: return gl::IndexType::u32;
		}
		ARC_ASSERT(false, "undhandled type");
		return gl::IndexType::Undefined;
	}

}} // namespace arc::renderer