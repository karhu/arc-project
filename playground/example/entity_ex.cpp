#include "example.hpp"

#include "arc/common.hpp"
#include "arc/memory/Allocator.hpp"

#include "../entity/entity.hpp"
#include "../component/TransformComponent.hpp"

#include <iostream>

void entity_example()
{
	using namespace arc;

	memory::Mallocator alloc;
	entity::Context world(&alloc, 1024);

	world.register_component<TransformComponent>(512);
	world.register_component<RenderComponent>(256);

	auto eh00 = world.create_entity();
	auto eh01 = world.create_entity();
	auto eh02 = world.create_entity();

	auto tc00 = world.add_component<TransformComponent>(eh00);
	auto tc01 = world.add_component<TransformComponent>(eh01);

	auto rc01 = world.add_component<RenderComponent>(eh01);
	auto rc02 = world.add_component<RenderComponent>(eh02);

	ARC_ASSERT(tc00.valid() && tc01.valid() && rc01.valid() && rc02.valid(), "components are invalid");

	tc01.set_position(vec3(1,2,3));
	tc01.set_scale(1.5f);
	
	rc01.set_visibility(true);
	rc01.set_color(vec4(1,0,0,1));

	tc01 = world.get_component<TransformComponent>(eh01);
	rc01 = world.get_component<RenderComponent>(eh01);

	ARC_ASSERT(tc00.valid() && tc01.valid() && rc01.valid() && rc02.valid(), "components are invalid");

	auto pos = tc01.get_position();
	auto col = rc01.get_color();

	std::cout << "position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
	std::cout << "color: " << col.r << " " << col.g << " " << col.b << " " << col.a << std::endl;

	// print all
	world.get_backend<TransformComponent>().update_all([&world](TransformComponent& self)
	{
		auto pos = self.get_position();
		std::cout << "<" << pos.x << ", " << pos.y << ", " << pos.z << ">";

		auto eh = self.entity();
		auto rc = world.get_component<RenderComponent>(eh);
		if (rc.valid()) std::cout << (rc.get_visibility() ? " + <visible>" : "+ <invisible>");

		std::cout << "\n";
	});
}