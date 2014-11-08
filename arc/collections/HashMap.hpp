#pragma once

#include "arc/core/numeric_types.hpp"
#include "arc/collections/Array.hpp"

namespace arc
{

	/// HashMap implementation
	/// based on: https://bitbucket.org/bitsquid/foundation
	template<typename T>
	class HashMap
	{
	public:
		struct Entry
		{
			uint64 key() { return m_key; }
			T& value() { return m_value; }
			const T& value() const { return m_value; }
		public:
			Entry(uint64_t key, const T& value, uint32_t next = 0)
				: m_key(key), m_value(value), m_next(next)
			{}

			Entry(uint64_t key, T&& value, uint32_t next = 0)
				: m_key(key), m_value(std::forward<T>(value)), m_next(next)
			{}

			Entry() : m_key(0), m_value(), m_next(0) {}
		public:
			Entry(Entry&& other)
				: m_key(other.m_key), m_value(std::move(other.m_value)), m_next(other.m_next)
			{}

			Entry& operator=(Entry&& other)
			{
				m_key = other.m_key;
				m_value = std::move(other.m_value);
				m_next = other.m_next;
				return *this;
			}

		private:
			uint64 m_key;
			T      m_value;
			uint32 m_next; 

			friend class HashMap<T>;
		};

	public:
		HashMap() = default;
		HashMap(memory::Allocator& alloc);

	public:
		void initialize(memory::Allocator* alloc);
		void finalize();
		bool is_initialized();

	public:
		Entry* lookup(uint64 key);
		const Entry* lookup(uint64 key) const;

		Entry& get(uint64 key, const T& fallback_init);
		Entry& get(uint64 key, const T& fallback_init, bool& o_fallback_used);

		Entry& get(uint64 key, T&& fallback_init);
		Entry& get(uint64 key, T&& fallback_init, bool& o_fallback_used);

		Entry& set(uint64 key, const T& value);

	public:
		bool remove(uint64 key);
		bool contains(uint64 key) const;

	public:
		void reserve(uint32 size);
		void clear();

	public:
		uint32 size() const;
		float  load_factor() const;
		uint32 collision_count() const;

	public: // range based for loop support
		Entry* begin();
		Entry* end();

		const Entry* begin() const;
		const Entry* end() const;

	private:
		struct FindResultPrev
		{
			uint32 h_idx;
			uint32 d_idx;
			uint32 prev_d_idx;
		};
		struct FindResult
		{
			uint32 h_idx;
			uint32 d_idx;
		};
	private:
		FindResultPrev find_prev(uint64 key) const;
		FindResult find(uint64 key) const;
		bool present(const FindResult& fr) const;
	private:
		void rehash(uint32 new_size);

	private:
		Array<uint32> m_hashes;
		Array<Entry>  m_data;
	private:
		static const uint32 END_INDEX = -1;

	};

} // namespace arc
