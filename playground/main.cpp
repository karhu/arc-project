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

int main(int argc, char** argv)
{
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

	std::cout << "<begin>" << std::endl;

	using PH = meta::enum_helper < gl::Primitive > ;
	std::cout << PH::type_name().c_str() << "::" << PH::value_name(PH::from_index(5)).c_str() << std::endl;



	arc::log::DefaultLogger default_logger;
	arc::log::set_logger(default_logger);

	// Struct etc ////////////

	memory::Mallocator malloc;

	SOA<Description> my_struct_of_arrays(&malloc, 512);

	auto& scale = my_struct_of_arrays.get<Description::SCALE>(12);


	// init engine /////////////////////////////////////////////////////////////////////
	engine::initialize(g_config);

	lua::State sys_config;
	engine::initialize_subsystems(sys_config);


	// experiment //////////////////////////////////////////////////////////////////////

	simple_mesh_example();
	//renderer_example();
	//entity_example();

	// stop engine /////////////////////////////////////////////////////////////////////

	engine::shutdown();

	// close ///////////////////////////////////////////////////////////////////////////

	std::cout << "<end>" << std::endl;
	system("pause");
	return 0;
}
