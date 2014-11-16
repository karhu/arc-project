#pragma once

#include "arc/common.hpp"
#include "arc/collections/Queue.hpp"
#include "arc/collections/Array.hpp"
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
		void initialize(memory::Allocator* alloc, uint32 last_id, uint32 increment, uint32 very_last_id, std::function<void(uint32)> increment_cb = nullptr);
		void finalize();
		bool is_initialized();
	public:
		uint32 create();
		void release(uint32 h);
		bool valid(uint32 idx) const;
	private:
		Queue<uint32> m_free_handles;
		uint32  m_next_id = 0;
		uint32  m_increment = 0;
		uint32  m_last_id = 0;
		uint32  m_very_last_id = 0;
		std::function<void(uint32)> m_resize_cb = nullptr;
		bool m_initialized = false;
	};

	template<typename T>
	class CompactPool
	{
	public:
		CompactPool(memory::Allocator* alloc, uint32_t initial_size, uint32_t increment, uint32_t max_size = (uint32_t)-1);
	public:
		arc::tuple<uint32_t, T*> create();
		void release(uint32_t idx);
		bool valid(uint32_t idx) const;
	public:
		T* data(uint32_t idx);
		const T* data(uint32_t idx) const;
	public:
		uint32_t find(std::function<bool(const T&)> cb);
	private:
		Array<uint32_t> m_indirection;
		Array<T> m_data;
		Array<uint32_t> m_back_indices;
		IndexPool32 m_indices;
	};

	// macro ///////////////////////////

	#define DECLARE_ID32(Name)					\
		struct _##Name##_IDTAG {};				\
		using Name = Index32<_##Name##_IDTAG>;


	// template implementation //////////////////////////

	template<typename T>
	CompactPool<T>::CompactPool(memory::Allocator* alloc, uint32_t initial_size, uint32_t increment, uint32_t max_size)
		: m_indirection(*alloc, initial_size + 1), m_data(*alloc), m_back_indices(*alloc)
	{
		m_indices.initialize(alloc, initial_size, increment, max_size - 1, [this](uint32 lastId) { m_indirection.resize(lastId + 1); });
	}

	template<typename T>
	tuple<uint32_t,T*> CompactPool<T>::create()
	{
		auto idx = m_indices.create();
		m_indirection[idx] = m_data.size();
		m_data.push_back();
		m_back_indices.push_back(idx);
		return std::make_tuple(idx, &m_data.back());
	}

	template<typename T>
	void CompactPool<T>::release(uint32_t idx)
	{
		ARC_ASSERT(valid(idx), "invalid index");
		uint32_t data_idx = m_indirection[idx];
		m_data[data_idx] = std::move(m_data.back());
		m_indirection[m_back_indices.back()] = data_idx;

		m_data.pop_back();
		m_back_indices.pop_back();

		m_indirection[idx] = (uint32_t)-1;
		m_indices.release(idx);
	}

	template<typename T>
	bool CompactPool<T>::valid(uint32_t idx) const
	{
		return m_indices.valid(idx) && m_indirection[idx] != (uint32_t)-1;
	}

	template<typename T>
	T* CompactPool<T>::data(uint32_t idx)
	{
		ARC_ASSERT(valid(idx), "invalid index");
		return &m_data[m_indirection[idx]];
	}

	template<typename T>
	const T* CompactPool<T>::data(uint32_t idx) const
	{
		ARC_ASSERT(valid(idx), "invalid index");
		return &m_data[m_indirection[idx]];
	}

	template<typename T>
	uint32_t CompactPool<T>::find(std::function<bool(const T&)> cb)
	{
		for (uint32_t i = 0; i < m_data.size(); i++)
		{
			if (cb(m_data[i])) return m_back_indices[i];
		}
		return 0;
	}
}