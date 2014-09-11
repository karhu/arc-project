#pragma once

#include "arc/common.hpp"
#include "arc/collections/Queue.hpp"
#include <functional>

namespace arc
{
	// handle ///////////////////////////

	template<typename TAG>
	struct Index32
	{
	public:
		inline uint32 value() const { return m_value; }
	public:
		Index32() = default;
		inline explicit Index32(uint32 v) : m_value(v) {}
	public:
		inline bool operator==(Index32<TAG> other) const { return m_value == other.m_value; }
		inline bool operator!=(Index32<TAG> other) const { return m_value != other.m_value; }
	private:
		uint32 m_value;
	};


	// pools ///////////////////////////

	class IndexPool32
	{
	public:
		void initialize(memory::Allocator* alloc, uint32 last_id, uint32 increment, uint32 very_last_id, std::function<void(uint32)> increment_cb);
		void finalize();
	public:
		uint32 create();
		void release(uint32 h);
		bool valid(uint32 idx);
	private:
		Queue<uint32> m_free_handles;
		uint32  m_next_id = 0;
		uint32  m_increment = 0;
		uint32  m_last_id = 0;
		uint32  m_very_last_id = 0;
		std::function<void(uint32)> m_resize_cb = nullptr;
	};

	// macro ///////////////////////////

	#define DECLARE_ID32(Name)					\
		struct _##Name##_IDTAG {};				\
		using Name = Index32<_##Name##_IDTAG>;
}