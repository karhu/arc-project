#pragma once

#include "arc/common.hpp"
#include "arc/math/vectors.hpp"
#include "arc/renderer/Renderer_GL44.hpp"
#include "arc/io/SimpleMesh.hpp"
#include "arc/gl/functions.hpp"
#include "arc/collections/Array.inl"

#include "../input/KeyboardState.hpp"


#include "../engine.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace arc;


#include "arc/math/common.hpp"
#include "arc/math/vector_math.hpp"

class Camera
{
public:
	mat4 compute_view_matrix() const
	{
		return glm::mat4_cast(m_forward)*glm::translate(mat4(), -m_world_pos);
	}

public: // moving the camera

	void translate_global(const vec3& delta)
	{
		m_world_pos += delta;
	}

	void translate_local(const vec3& delta)
	{
		m_world_pos += glm::mat3_cast(glm::inverse(m_forward))*delta;
	}

	void rotate_global(const vec3& axis, float deg)
	{
		float a = deg2rad(deg);
		vec3 ax = math::normalize(axis);

		float sina = sin(a / 2.0f);
		float cosa = cos(a / 2.0f);

		quat offset(cosa, ax*sina);

		// right multiplication corresponds to a global rotation
		m_forward = m_forward*offset;
	}

	void rotate_local(const vec3& axis, float deg)
	{
		float a = deg2rad(deg);
		vec3 ax = math::normalize(axis);

		float sina = sin(a / 2.0f);
		float cosa = cos(a / 2.0f);

		quat offset(cosa, ax*sina);

		// left multiplication corresponds to a local rotation
		m_forward = offset*m_forward;
	}
public:
	Camera& set_position(const vec3& pos)
	{
		m_world_pos = pos; return *this;
	}
public:
	vec3 get_position() const { return m_world_pos; }
protected:
	vec3 m_world_pos = vec3(0, 0, 0);
	quat m_forward = quat();
};

class PerspectiveCamera : public Camera
{
public:
	mat4 compute_projection_matrix() const
	{
		return glm::perspective(m_fov, m_aspect, m_near, m_far);
	}
public:
	PerspectiveCamera& set_near_far(float near, float far) { m_near = near; m_far = far; return *this; }
	PerspectiveCamera& set_fov(float fov) { m_fov = fov; return *this; }
	PerspectiveCamera& set_aspect(float aspect) { m_aspect = aspect; return *this; }
public:
	float get_near() const { return m_near; }
	float get_far() const { return m_far; }
	float get_fov() const { return m_fov; }
	float get_aspect() const { return m_aspect; }
protected:
	float m_aspect = 16.0f / 9.0f; // width / height
	float m_fov = 45.0f;
	float m_near = 0.1f;
	float m_far = 100.0f;
};

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
	bool stop = false;
	//using namespace arc::input;

	PerspectiveCamera cam;
	input::KeyboardState kb;

	glEnable(GL_DEPTH_TEST);

	while (!stop)
	{
		gl::clear_color(0.55f, 0.55f, 0.65f, 1.0f);
		gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
		glViewport(0, 0, 1280, 720);

		engine::deprecated_update();
		r.update_frame_begin();
		kb.update_frame_begin();

		int32_t count = 5;
		float spacing = 2.5f;

		// submit
		auto rb = r.render_bucket_create(count*count*count, count*count*count * 40 * sizeof(float));
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

		auto transform_view_proj_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::mat4x4_t,
			SH32("transform.view_proj"));
		_CHECK(color_offset != -1, "shader_get_uniform_offset unsuccessful");

		vec3 colors[] = {
			vec3(0.3f, 0.6f, 0.9f),
			vec3(0.6f, 0.9f, 0.3f),
			vec3(0.9f, 0.3f, 0.6f),
		};

		for (int32 i = 0; i < count; i++)
		{
			for (int32 j = 0; j < count; j++)
			{
				for (int32 k = 0; k < count; k++)
				{
					mat4 transform = glm::translate(mat4(), vec3(i*spacing, j*spacing, k*spacing));

					auto buffer = rb->add(id_shader, geometries.back(), 0);
					_CHECK(buffer.valid(), "draw unsuccessful");
					buffer.access<vec3>(color_offset) = colors[i % 3];
					buffer.access<mat4>(transform_model_offset) = transform;
					buffer.access<mat4>(transform_view_proj_offset) = cam.compute_projection_matrix() * cam.compute_view_matrix();
						//glm::perspective(45.0f, 16.0f / 9.0f, 0.1f, 100.0f) * glm::lookAt(-5*spacing*vec3(1, 1, 1), vec3(0, 0, 0), vec3(0, 1, 0));// *cam.compute_view_matrix();
				}
			}
		}
		r.render_bucket_submit(rb);


		r.update_frame_end();
		engine::deprecated_swap();

		// check for keyboard input
		using namespace input;
		if (kb.down(Key::Escape)) stop = true;

		float cam_translate_speed = 0.1f;
		float cam_rotate_speed = 1.0f;

		if (kb.down(Key::W)) cam.translate_local(vec3(0, 0, -cam_translate_speed));
		if (kb.down(Key::S)) cam.translate_local(vec3(0, 0, cam_translate_speed));
		if (kb.down(Key::D)) cam.translate_local(vec3(cam_translate_speed, 0, 0));
		if (kb.down(Key::A)) cam.translate_local(vec3(-cam_translate_speed, 0, 0));
		if (kb.down(Key::R)) cam.translate_local(vec3(0,cam_translate_speed, 0));
		if (kb.down(Key::F)) cam.translate_local(vec3(0,-cam_translate_speed, 0));

		if (kb.down(Key::E)) cam.rotate_global(vec3(0, 1, 0), cam_rotate_speed);
		if (kb.down(Key::Q)) cam.rotate_global(vec3(0, 1, 0),-cam_rotate_speed);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	std::cout << "<simple_mesh_ex_end>" << "\n" << std::endl;
}
