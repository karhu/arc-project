#pragma once

#include "Allocator.hpp"

namespace arc { namespace memory {

	class LinearAllocator : public Allocator
	{
	public:
		LinearAllocator() = default;
		~LinearAllocator();
	public:
		void initialize(memory::Allocator* parent, uint32 size);
		void finalize();
	public:
		void* allocate(uint64 size, uint32 align = 4) override;
		void  free(void* data) override;
	public:
		void reset();
	private:
		memory::Allocator* m_parent_alloc = nullptr;
		char*  m_begin = nullptr;
		char*  m_end = nullptr;
		char*  m_current = nullptr;
	};


}}