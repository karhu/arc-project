#pragma once

#include "arc/core/numeric_types.hpp"

namespace arc
{
	struct Counter32
	{
	public:
		Counter32() = default;
		Counter32(uint32 value) : m_value(value) {}
	public:
		inline uint32 value() const { return m_value; }
		inline uint32 set(uint32 new_value) { m_value = new_value; }
	public:
		inline void increment() { m_value += 1; }
		inline void decrement() { m_value -= 1; }
	private:
		uint32 m_value;
	};


	inline bool operator<(Counter32 lh, Counter32 rh) 
	{
		static const uint32 HALF_RANGE = 2 ^ 31;
		uint32 lv = lh.value();
		uint32 rv = rh.value();

		return ((lv < rv) && ((rv - lv) < HALF_RANGE)) || ((rv < lv) && ((lv - rv) > HALF_RANGE));
	}

	inline bool operator>(Counter32 lh, Counter32 rh)
	{
		return rh < lh;
	}

	inline bool operator==(Counter32 lh, Counter32 rh) 
	{
		return lh.value() == rh.value();
	}
}