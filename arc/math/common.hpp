#pragma once

namespace arc
{
	namespace math
	{
		const float PI = 3.14159265359f;
		const float INV_PI = 0.31830988618f;
	}
}

namespace arc
{
	template <typename T, typename U> inline
	T min(T v1, U v2) 
	{ 
		return v1 < v2 ? v1 : v2; 
	}

	template <typename T> inline
	T min(T v1, T v2, T v3) 
	{ 
		T a = min(v1, v2);
		return min(a, v3);
	}

	template <typename T> inline
	T min(T v1, T v2, T v3, T v4)
	{
		T a = min(v1, v2);
		T b = min(v3, v4);
		return min(a, b);
	}

}

namespace arc
{
	template <typename T, typename U> inline
	T max(T v1, U v2) 
	{ 
		return v1 > v2 ? v1 : v2; 
	}

	template <typename T> inline
	T max(T v1, T v2, T v3)
	{
		T a = max(v1, v2);
		return max(a, v3);
	}

	template <typename T> inline
	T max(T v1, T v2, T v3, T v4)
	{
		T a = max(v1, v2);
		T b = max(v3, v4);
		return max(a, b);
	}

}

namespace arc
{
	inline float deg2rad(float angle) { return  angle * math::PI * (1.0f/180.0f); }
	inline float rad2deg(float angle) { return  angle * math::INV_PI * 180.0f;  }
}