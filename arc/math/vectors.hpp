#pragma once

#include "glm/glm.hpp"
#include "glm/core/type.hpp"

#include "arc/core/numeric_types.hpp"

namespace arc
{
	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;

	using vec2u8 = glm::detail::tvec2<uint8>;
	using vec3u8 = glm::detail::tvec3<uint8>;
	using vec4u8 = glm::detail::tvec4<uint8>;

	using vec2i8 = glm::detail::tvec2<int8>;
	using vec3i8 = glm::detail::tvec3<int8>;
	using vec4i8 = glm::detail::tvec4<int8>;

	using vec2u16 = glm::detail::tvec2<uint16>;
	using vec3u16 = glm::detail::tvec3<uint16>;
	using vec4u16 = glm::detail::tvec4<uint16>;

	using vec2i16 = glm::detail::tvec2<int16>;
	using vec3i16 = glm::detail::tvec3<int16>;
	using vec4i16 = glm::detail::tvec4<int16>;

	using vec2u32 = glm::detail::tvec2<uint32>;
	using vec3u32 = glm::detail::tvec3<uint32>;
	using vec4u32 = glm::detail::tvec4<uint32>;

	using vec2i32 = glm::detail::tvec2<int32>;
	using vec3i32 = glm::detail::tvec3<int32>;
	using vec4i32 = glm::detail::tvec4<int32>;

	using mat3 = glm::detail::tmat3x3<float>;
	using mat4 = glm::detail::tmat4x4<float>;
}