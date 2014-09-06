#pragma once

#include "arc/common.hpp"

namespace arc
{

	template<typename T>
	class Queue
	{
	public:
		Queue(memory::Allocator* alloc, uint32 capacity = 8);
		Queue() = default;
	public:
		inline ~Queue() { finalize(); }
	public:
		void initialize(memory::Allocator* alloc, uint32 capacity = 8);
		void finalize();
		bool is_initialized();
	public:
		T& front();
		T& back();
	public:
		T pop_front();
		T pop_back();
	public:
		void remove_front();
		void remove_back();
	public:
		void push_front(const T& value);
		void push_back(const T& value);
	public:
		void grow(uint32 additional_capacity);
	public:
		uint32 size() const;
		uint32 capacity() const;
		bool empty() const;
	private:
		uint32 last_index() const;
	private:
		T* m_data = nullptr;
		uint32 m_capacity = 0;
		uint32 m_size  = 0;
		uint32 m_first = 0;
		memory::Allocator* m_alloc = nullptr;
	};

} // namespace arc
