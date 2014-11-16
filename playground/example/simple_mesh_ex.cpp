#pragma once

#include "arc/common.hpp"
#include "arc/math/vectors.hpp"
#include "arc/renderer/Renderer_GL44.hpp"
#include "arc/io/SimpleMesh.hpp"
#include "arc/gl/functions.hpp"
#include "arc/collections/Array.inl"

#include "../engine.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace arc;

#define _CHECK(condition,message) if (!(condition)) { std::cout << "FAILURE: " << message << std::endl; return; }

inline void simple_mesh_example()
{
	using namespace arc::renderer;

	std::cout << "<simple_mesh_ex_begin>" << std::endl;

	// variables
	ShaderID id_shader;

	// memory allocator
	memory::Mallocator longterm_alloc;

	// meshes
	Array<renderer::GeometryID> geometries(longterm_alloc);

	// renderer configuration
	renderer::Config config;
	config.longterm_allocator = &longterm_alloc;

	// initialize renderer
	bool ok = Renderer_GL44::Validate(config);
	_CHECK(ok, "invalid renderer config");
	Renderer_GL44 renderer_gl44(config);
	auto& r = renderer_gl44;

	// create shader
	id_shader = r.shader_create("../../simple_mesh_ex_shader.lua");
	_CHECK(id_shader != INVALID_SHADER_ID, "invalid ShaderID");

	io::FileReadStream in; 
	ok = in.open("../../icoshphere_from_obj.sm.arc");
	_CHECK(ok, "mesh file not found");

	ok = io::load_simple_mesh_to_gpu(r, in, [&](arc::renderer::GeometryID id) { geometries.push_back(id); });
	_CHECK(ok, "mesh load error");
	_CHECK(!geometries.empty(), "no geomeries loaded");

	// main loop //

	int32 counter = 60;
	bool stop = false;
	//using namespace arc::input;

	while ((counter > 0 && !stop))
	{
		gl::clear_color(0.55f, 0.55f, 0.65f, 1.0f);
		gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
		glViewport(0, 0, 1280, 720);
		engine::deprecated_update();
		r.update_frame_begin();

		// submit
		auto rb = r.render_bucket_create(25, 25 * 20 * sizeof(float));
		_CHECK(rb != nullptr, "invalid RenderBucket");

		auto color_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::vec3_t,
			SH32("instance.color"));
		_CHECK(color_offset != -1, "shader_get_uniform_offset unsuccessful");

		auto transform_model_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::mat4x4_t,
			SH32("transform.model"));
		_CHECK(color_offset != -1, "shader_get_uniform_offset unsuccessful");


		vec3 colors[] = {
			vec3(0.3f, 0.6f, 0.9f),
			vec3(0.6f, 0.9f, 0.3f),
			vec3(0.9f, 0.3f, 0.6f),
		};

		mat4 transform;
		vec3 translation{ 1, 1, 0 };
		translation *= 0.3f;

		for (uint32 i = 0; i < 20; i++)
		{
			auto buffer = rb->add(id_shader, geometries.back(), 0);
			_CHECK(buffer.valid(), "draw unsuccessful");
			buffer.access<vec3>(color_offset) = colors[i % 3];
			buffer.access<mat4>(transform_model_offset) = transform;
			transform = glm::translate(transform, translation)*glm::scale(mat4(),vec3(2,2,2));
		}
		r.render_bucket_submit(rb);


		r.update_frame_end();
		engine::deprecated_swap();

		//if (keyboard.down(Key::Escape)) stop = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		--counter;
	}

	std::cout << "<simple_mesh_ex_end>" << "\n" << std::endl;
}
