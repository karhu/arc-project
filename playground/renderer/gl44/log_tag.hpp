#pragma once

#include "arc/logging/log.hpp"

namespace arc { namespace renderer { namespace gl44 {

	struct tag_gl44
	{
		static const char* name() { return "Renderer_GL44"; }
		static int priority() { return log::PRIORITY_INFO; }
	};

}}}