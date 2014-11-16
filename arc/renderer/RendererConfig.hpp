#pragma once

#include "arc/common.hpp"

namespace arc { namespace renderer {

	struct Config
	{
		memory::Allocator* longterm_allocator = nullptr;

		uint32 geometry_buffer_static_size = 64 * 1024 * 1024; // 64MB
		uint32 frame_allocator_size = 2 * 1024 * 1024;		   // 2MB
	};

}} // namespace arc::renderer
