#include "example.hpp"

#include "arc/common.hpp"
#include "arc/memory/Allocator.hpp"

#include "../entity2/entity2.hpp"
#include "../entity2/TransformComponent2.hpp"

#include <iostream>

void entity_example()
{
	using namespace arc;

	memory::Mallocator alloc;
	entity2::Context world(&alloc, 256);

	world.register_component<entity2::TransformComponent>(256);
	world.register_component<entity2::RenderComponent>(128);

	auto eh00 = world.create_entity();
	auto eh01 = world.create_entity();
	auto eh02 = world.create_entity();

	auto tc00 = world.add_component<entity2::TransformComponent>(eh00);
	auto tc01 = world.add_component<entity2::TransformComponent>(eh01);

	auto rc01 = world.add_component<entity2::RenderComponent>(eh01);
	auto rc02 = world.add_component<entity2::RenderComponent>(eh02);

	ARC_ASSERT(tc00.valid() && tc01.valid() && rc01.valid() && rc02.valid(), "components are invalid");

	tc01.set_position(vec3(1,2,3));
	tc01.set_scale(1.5f);
	
	rc01.set_visibility(true);
	rc01.set_color(vec4(1,0,0,1));

	tc01 = world.get_component<entity2::TransformComponent>(eh01);
	rc01 = world.get_component<entity2::RenderComponent>(eh01);

	ARC_ASSERT(tc00.valid() && tc01.valid() && rc01.valid() && rc02.valid(), "components are invalid");

	auto pos = tc01.get_position();

	std::cout << "position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
	auto col = rc01.get_color();
	std::cout << "color: " << col.r << " " << col.g << " " << col.b << " " << col.a << std::endl;
}