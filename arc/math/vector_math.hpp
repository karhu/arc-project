#pragma once

#include "vectors.hpp"

namespace arc { namespace math {

	inline float dot(const vec3& a, const vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

	inline vec3 normalize(const vec3& v) 
	{ 
		float len = sqrt(dot(v, v));
		return v * (1.0f / len);
	}

	inline quat normalize(const quat& v) 
	{
		return glm::normalize(v);
	}
}}