// main.cpp : Defines the entry point for the console application.
//


#include <iostream>
#include <algorithm>

#include "arc/core.hpp"
#include "arc/memory/Allocator.hpp"
#include "arc/logging/log.hpp"
#include "arc/string/util.hpp"

#include "engine.hpp"

#include "arc/gl/functions.hpp"
#include "arc/gl/meta.hpp"
#include "arc/math/vectors.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <boost/iterator/zip_iterator.hpp>

#include "arc/memory/util.hpp"

#include "arc/renderer/RendererBase.hpp"

using namespace arc;

engine::Config g_config;

#include "example/renderer_ex.hpp"
#include "example/example.hpp"
#include "Struct.hpp"

#include "renderer/test.hpp"

int main(int argc, char** argv)
{
	std::cout << "<begin>" << std::endl;

	// run experiments
	simple_mesh_example();
	//simple_mesh_example();
	//renderer_example();
	//entity_example();
	texture_example();

	std::cout << "<end>" << std::endl;
	system("pause");
	return 0;
}


void old_tests(int argc, char** argv)
{
	// extracting working folder from argv
	std::cout << argv[0] << std::endl;

	StringView executable(argv[0], 0, arc::cstr::length(argv[0], 1024));
	StringView path = string::remove_trailing(executable, [](char c) {
		switch (c)
		{
		case '/': return false;
		case '\\': return false;
		default: return true;
		}
	});

	std::cout.write(path.c_str(), path.length());
	std::cout << std::endl;

	// enum meta data test
	using PH = meta::enum_helper < gl::Primitive >;
	std::cout << PH::type_name().c_str() << "::" << PH::value_name(PH::from_index(5)).c_str() << std::endl;

	// struct of array test
	memory::Mallocator malloc;
	SOA<Description> my_struct_of_arrays(&malloc, 512);

	auto& scale = my_struct_of_arrays.get<Description::SCALE>(12);


	// render queue v2 

	uint32_t buffer_size = 1024 * 1024 * 8;
	void* buffer = malloc.allocate(buffer_size); // 8MB
	RenderBucket<DrawCommand> renderbucket(buffer, memory::util::ptr_add(buffer, buffer_size));

	auto cmd_queue_00 = renderbucket.create_queue(126, 96);
	for (uint32_t i = 0; i < 126; i++)
	{
		auto cmd = cmd_queue_00->create_draw_command(ShaderID());
		ARC_ASSERT(cmd.valid(), "invalid cmd");
		cmd.set_geometry(GeometryID());
		auto sh = cmd.get_shader_storage();
		ARC_ASSERT(sh.size == 96, "invalid shader storage size");
		cmd_queue_00->submit_draw_command(cmd);
	}
	renderbucket.submit_queue(cmd_queue_00);
	renderbucket.sort();

	for (auto cmd : renderbucket.get_commands())
	{
		auto key = cmd.key;
		auto data = cmd.data;
		std::cout << key << " : " << data << " : " << data->m_shader.value() << std::endl;
	}
}