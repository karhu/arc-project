#pragma once

#include "arc/common.hpp"

namespace arc { namespace renderer {

	struct Config
	{
		uint32 geometry_buffer_static_size = 64 * 1024 * 1024; // 64MB
		uint32 frame_allocator_size = 4 * 1024 * 1024;		   // 4MB
	};

	struct AllocatorConfig
	{
		memory::Allocator* longterm_allocator = nullptr;
	};

}} // namespace arc::renderer
