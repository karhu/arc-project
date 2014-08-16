#pragma once

#include "arc/core.hpp"
#include "arc/collections/Slice.hpp"

#include "arc/memory/util.hpp"

namespace arc
{
	namespace memory{ class Allocator; }

	template<typename T>
	class Array
	{
	public:
		Array(memory::Allocator& a, uint32 size = 0);
		Array();
		~Array();

		ARC_NO_COPY(Array);

	public:
		T& operator[] (uint32 idx);
		const T& operator[] (uint32 idx) const;
		
	public:
		uint32 size() const;
		bool empty() const;

	public:
		T* data();
		const T* data() const;

	public:
		void push_back(const T& value = T());
		void pop_back();
		T& back();
		const T& back() const;

	public:
		template<typename ...Args>
		void emplace_back(Args&& ...args);

	public:
		void resize(uint32 size);
		void resize(uint32 size, const T& init_value);

		void reserve(uint32 size);
		void trim();
		void clear();

	public:
		void initialize(memory::Allocator* alloc, uint32 size = 0);
		void finalize();
		bool is_initialized();

	protected:
		void _grow(uint32 minCapacity = 0);

	protected:
		memory::Allocator* m_allocator = nullptr;
		T*                 m_data = nullptr;
		uint32             m_size = 0;
		uint32             m_capacity = 0;
	};

	// slice functionality ///////////////////////////////////////////////////////////////

	template<typename T> ARC_CONSTEXPR
	inline Slice<T> make_slice(Array<T>& a)
	{
		return make_slice(a.data(), a.size());
	}

	template<typename T> ARC_CONSTEXPR
	inline const Slice<T> make_slice(const Array<T>& a)
	{
		return make_slice(a.data(), a.size());
	}

	template<typename T> inline
	Array<T>::~Array()
	{
		finalize();
	}
}