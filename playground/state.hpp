#pragma once

#include "arc/common.hpp"


#include "arc/gl/types.hpp"
namespace arc
{
	struct HardcodedGLStuff
	{
		uint32 di_buffer;
		void*  di_data;
		uint32 vao;
		uint32 ea_buffer;
		uint32 vb_binding_idx;
		uint32 vb;
		uint32 vb_stride;

		uint32 program;
		uint32 di_cmd_count;
	};

	extern HardcodedGLStuff hc_gl_stuff;

	void dbg_print(gl::DrawElementsIndirectCommand* cmd, uint32 count);

}