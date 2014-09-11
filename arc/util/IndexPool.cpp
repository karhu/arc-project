#include "IndexPool.hpp"

#include "arc/collections/Queue.inl"

namespace arc
{
	void IndexPool32::initialize(memory::Allocator* alloc, uint32 last_id, uint32 increment, uint32 very_last_id, std::function<void(uint32)> increment_cb)
	{
		m_free_handles.initialize(alloc, 16);
		m_next_id = 1;
		m_last_id = last_id;
		m_increment = increment;
		m_very_last_id = very_last_id;
		m_resize_cb = increment_cb;
	}

	void IndexPool32::finalize()
	{
		m_free_handles.finalize();

		m_next_id = 0;
		m_very_last_id = 0;
		m_increment = 0;
		m_last_id = 0; 
		m_resize_cb = nullptr;
	}

	uint32 IndexPool32::create()
	{
		// recycle used handles
		if (!m_free_handles.empty())
		{
			uint32 h = m_free_handles.front() ;
			m_free_handles.pop_front();
			return h;
		}

		// check whether "resize" is necessary
		if (m_next_id > m_last_id)
		{
			if (m_next_id > m_very_last_id) return 0;

			m_last_id += m_increment;
			m_last_id = std::min(m_last_id, m_very_last_id);

			m_resize_cb(m_last_id);
		}

		return m_next_id++;
	}

	void IndexPool32::release(uint32 h)
	{
		if (h == 0) return;
		m_free_handles.push_back(h);
	}

	bool IndexPool32::valid(uint32 idx)
	{
		return idx > 0 && idx < m_next_id;
	}
}

/*
	template<uint8 INDEX_BITS, uint8 GENERATION_BITS>
	struct Handle32G
	{
		uint32 m_index : INDEX_BITS;
		uint32 m_generation : GENERATION_BITS;

		static_assert(INDEX_BITS + GENERATION_BITS <= 32, "Total bit count exceeds 32.");
	};

	template<uint8 INDEX_BITS, uint8 GENERATION_BITS, uint8 CUSTOM_BITS>
	struct Handle32GC
	{
		uint32 m_index : INDEX_BITS;
		uint32 m_generation : GENERATION_BITS;
		uint32 m_custom : CUSTOM_BITS;

		static_assert(INDEX_BITS + GENERATION_BITS + CUSTOM_BITS <= 32, "Total bit count exceeds 32.");
	};
*/

/*
namespace arc
{
	template<uint8 INDEX_BITS, uint8 GENERATION_BITS>
	class HandlePool32
	{
	public:
		using Handle = Handle32<INDEX_BITS, GENERATION_BITS>;
	public:
		void initialize(memory::Allocator* alloc, uint32 size, uint32 increment, std::function<void(uint32)> resize_cb);
		void finalize();
	public:
		Handle create();
		bool destroy(Handle h);
		bool valid(Handle h);
	private:
		void grow();
	private:
		static const uint32 INVALID_INDEX = 0;
	private:
		Handle* m_data = nullptr;
		uint32  m_first_free = INVALID_INDEX;
		uint32  m_last_free = INVALID_INDEX;

		memory::Allocator* m_alloc = nullptr;
		uint32  m_size = 0;
		uint32  m_increment = 0;
		std::function<void(uint32)> m_resize_cb = nullptr;
	};

	template<uint8 I, uint8 G>
	void HandlePool32<I, G>::finalize()
	{
		if (m_data != nullptr)
		{
			m_alloc->free(m_data);

			m_data = nullptr;
			m_first_free = INVALID_INDEX;
			m_last_free = INVALID_INDEX;
			m_alloc = nullptr;
			m_size = 0;
			m_increment = 0;

			m_resize_cb(0);
			m_resize_cb = nullptr;
		}
	}

	template<uint8 I, uint8 G>
	void HandlePool32<I, G>::initialize(memory::Allocator* alloc, uint32 size, uint32 increment, std::function<void(uint32)> resize_cb)
	{
		finalize();

		m_alloc = alloc;
		m_size = size+1;
		m_increment = grow_increment == 0 ? 1 : grow_increment;
		m_resize_cb = resize_cb;

		m_data = (Handle*)m_alloc->allocate(sizeof(Handle)*m_size, alignof(Handle));

		// init free list
		m_first_free = 1;
		m_last_free = m_size - 1;

		for (uint32 i = 0; i < m_size; i++)
		{
			m_data[i].m_index = i+1;
		}
		m_data[m_size - 1].m_index = INVALID_INDEX;
		m_data[0].m_index = INVALID_INDEX;
	}

	template<uint8 I, uint8 G>
	Handle32<I, G> HandlePool32<I, G>::create()
	{
		if (m_first_free = INVALID_INDEX) grow();

		auto& data = m_data[m_first_free];
		Handle32<I, G> h{ m_first_free, data.m_generation };

		// pop front of free list
		m_first_free = data.m_index;

		return h;
	}

	template<uint8 I, uint8 G>
	bool HandlePool32<I, G>::destroy(Handle32<I,G> h)
	{
		if (valid(h))
		{
			m_data[m_last_free].m_index = h.m_index;
			m_last_free = h.m_index;
			m_data[m_last_free].m_index = INVALID_INDEX;
			m_data[m_last_free].m_generation += 1;
			return true;
		}
		return false;
	}

	template<uint8 I, uint8 G>
	bool HandlePool32<I, G>::valid(Handle32<I, G> h)
	{
		if (h.m_index >= m_size) return false;
		return m_data[h.m_index].generation == h.m_generation;
	}

	template<uint8 I, uint8 G>
	void HandlePool32<I, G>::grow()
	{
		uint32 new_size = m_size + m_increment;
		Handle* new_data = (Handle*)m_alloc->allocate(sizeof(Handle)*new_size, alignof(Handle));

		memory::util::move_elements<Handle>(m_data, new_data, m_size);

		// init free list
		for (uint32 i = m_size; i < new_size; i++)
		{
			m_data[i].m_index = i + 1;
		}
		m_data[m_size - 1] = INVALID_INDEX;

		if (m)
	}
}
*/
