
#include "arc/common.hpp"
#include "../engine//SimpleMainLoop.hpp"
#include "arc/io/SimpleMesh.hpp"

#include "arc/math/common.hpp"
#include "arc/math/vectors.hpp"
#include "arc/math/vector_math.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace arc;

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

void simple_mesh_example()
{
	std::cout << "<simple_mesh_ex_begin>" << std::endl;

	SimpleMainLoop mainloop;

	// variables
	renderer::ShaderID id_shader;
	Array<renderer::GeometryID> geometries(mainloop.longterm_allocator());
	PerspectiveCamera cam;

	const vec3 colors[] = {
		vec3(0.3f, 0.6f, 0.9f),
		vec3(0.6f, 0.9f, 0.3f),
		vec3(0.9f, 0.3f, 0.6f),
	};

	const vec3 colors2[] = {
		vec3(0.3f, 0.6f, 0.9f),
		vec3(0.3f, 0.9f, 0.6f),
		vec3(0.6f, 0.9f, 0.3f),
		vec3(0.6f, 0.3f, 0.9f),
		vec3(0.9f, 0.3f, 0.6f),
		vec3(0.9f, 0.6f, 0.3f),
	};

	const int32_t COUNT = 13;
	const float SPACING = 2.5f;

	mainloop.set_initialize_function([&](SimpleMainLoop::Context context)
	{
		using namespace arc::renderer;
		auto& r = context.renderer;
		bool ok;

		// load shader
		id_shader = r.shader_create("../../../resources/simple_mesh_ex/simple_mesh_ex_shader.lua");
		if (id_shader == INVALID_SHADER_ID) { ARC_ASSERT(false, "invalid ShaderID"); return false; }
		
		// load mesh
		io::FileReadStream in;
		ok = in.open("../../../resources/simple_mesh_ex/icoshphere_from_obj.sm.arc");
		if (!ok) { ARC_ASSERT(false, "mesh file not found"); return false; }
		ok = io::load_simple_mesh_to_gpu(r, in, [&](arc::renderer::GeometryID id) { geometries.push_back(id); });
		if (!ok) { ARC_ASSERT(false, "mesh load error"); return false; }
		if (geometries.empty()) { ARC_ASSERT(false, "no geomeries loaded"); return false; }

		// TODO workaround for now
		glEnable(GL_DEPTH_TEST);

		return true;
	});

	mainloop.set_update_function([&](SimpleMainLoop::Context& context, double dt)
	{
		using namespace input;
		auto& kb = context.keyboard;

		if (kb.down(Key::Escape)) return SimpleMainLoop::Status::STOP;

		float dtf = float(dt);
		float cam_translate_speed = dtf*6.0f;
		float cam_rotate_speed = dtf*30.0f;

		if (kb.down(Key::W)) cam.translate_local(vec3(0, 0, -cam_translate_speed));
		if (kb.down(Key::S)) cam.translate_local(vec3(0, 0, cam_translate_speed));
		if (kb.down(Key::D)) cam.translate_local(vec3(cam_translate_speed, 0, 0));
		if (kb.down(Key::A)) cam.translate_local(vec3(-cam_translate_speed, 0, 0));
		if (kb.down(Key::R)) cam.translate_local(vec3(0, cam_translate_speed, 0));
		if (kb.down(Key::F)) cam.translate_local(vec3(0, -cam_translate_speed, 0));

		if (kb.down(Key::E)) cam.rotate_global(vec3(0, 1, 0), cam_rotate_speed);
		if (kb.down(Key::Q)) cam.rotate_global(vec3(0, 1, 0), -cam_rotate_speed);

		return SimpleMainLoop::Status::CONTINUE;
	});

	mainloop.set_render_function([&](SimpleMainLoop::Context& context, double dt)
	{
		using namespace arc::renderer;
		auto& r = context.renderer;

		// viewport reset and setup
		gl::clear_color(0.55f, 0.55f, 0.65f, 1.0f);
		gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
		glViewport(0, 0, context.engine_config.window_width, context.engine_config.window_height);

		// query shader uniform locations;
		auto color_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::vec3_t,
			SH32("instance.color"));
		if (color_offset == -1) { ARC_ASSERT(false, "shader_get_uniform_offset unsuccessful"); return SimpleMainLoop::Status::ERROR; }

		auto transform_model_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::mat4x4_t,
			SH32("transform.model"));
		if (color_offset == -1) { ARC_ASSERT(false, "shader_get_uniform_offset unsuccessful"); return SimpleMainLoop::Status::ERROR; }

		auto transform_view_proj_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::mat4x4_t,
			SH32("transform.view_proj"));
		if (color_offset == -1) { ARC_ASSERT(false, "shader_get_uniform_offset unsuccessful"); return SimpleMainLoop::Status::ERROR; }

		// submit draw calls
		auto rb = r.render_bucket_create(COUNT*COUNT*COUNT, COUNT*COUNT*COUNT * 40 * sizeof(float));
		if (rb == nullptr) { ARC_ASSERT(false, "invalid RenderBucket"); return SimpleMainLoop::Status::ERROR; }

		for (int32 i = 0; i < COUNT; i++)
		{
			for (int32 j = 0; j < COUNT; j++)
			{
				for (int32 k = 0; k < COUNT; k++)
				{ 
					mat4 transform = glm::translate(mat4(), vec3(i, j, k)*SPACING);

					auto buffer = rb->add(id_shader, geometries.back(), 0);
					if (!buffer.valid()) { ARC_ASSERT(false, "draw unsuccessful"); return SimpleMainLoop::Status::ERROR; }

					buffer.access<vec3>(color_offset) = colors2[(i*COUNT*COUNT + j*COUNT + k) % 6];
					buffer.access<mat4>(transform_model_offset) = transform;
					buffer.access<mat4>(transform_view_proj_offset) = cam.compute_projection_matrix() * cam.compute_view_matrix();
				}
			}
		}

		r.render_bucket_submit(rb);

		return SimpleMainLoop::Status::CONTINUE;
	});

	auto ret = mainloop.run();
	if (ret == SimpleMainLoop::Status::ERROR)
	{
		ARC_ASSERT(false, "Execution stopped due to an error");
	}

	std::cout << "<simple_mesh_ex_end>" << "\n" << std::endl;
}
