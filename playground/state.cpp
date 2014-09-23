

#include "state.hpp"
#include <iostream>


namespace arc
{
	HardcodedGLStuff hc_gl_stuff;

	void dbg_print(gl::DrawElementsIndirectCommand* cmd, uint32 count)
	{
		for (uint32 i = 0; i < count; i++)
		{
			auto& c = cmd[i];

			std::cout << "[ count=" << c.count << ", i_count=" << c.instance_count << ", first_index=" << c.first_index << ", base_vertex=" << c.base_vertex << ", base_instance=" << c.base_instance
				<< " ]" << std::endl;
		}
	}
}


/*
struct DrawElementsIndirectCommand
{
GLuint count;
GLuint instance_count;
GLuint first_index;
GLuint base_vertex;
GLuint base_instance;
};
*/