

#include "arc/common.hpp"
#include "../engine//SimpleMainLoop.hpp"
#include "arc/renderer/gl44/texture.hpp"

#include "arc/math/common.hpp"
#include "arc/math/vectors.hpp"
#include "arc/math/vector_math.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

using namespace arc;

struct TexExampleVertexData
{
	vec3   position;
	vec2   uv;

	static renderer::VertexLayout& Layout();
};

renderer::VertexLayout& TexExampleVertexData::Layout()
{
	using namespace arc::renderer;
	using Type = VertexAttribute::Type;
	static VertexAttribute attributes[] = {
			{ SH32("position"), Type::float32, 3, offsetof(TexExampleVertexData, position) },
			{ SH32("uv"), Type::float32, 2, offsetof(TexExampleVertexData, uv) },
	};
	static VertexLayout layout = { SH32("TexExampleVertexData"), attributes, 2, sizeof(TexExampleVertexData) };
	return layout;
}

void texture_example()
{
	using namespace arc::renderer;

	std::cout << "<texture_example_begin>" << std::endl;

	// settings
	const uint32_t DIM = 256;
	const uint32_t DIM2 = 128;

	// variables
	SimpleMainLoop mainloop;
	ShaderID id_shader;
	GeometryID id_geometry_plane;
	GeometryConfigID id_geometry_config;
	gl44::TextureHandle id_tex, id_tex_2;

	// memory allocator
	memory::Mallocator longterm_alloc;

	// meshes
	Array<renderer::GeometryID> geometries(mainloop.longterm_allocator());

	
	mainloop.set_initialize_function([&](SimpleMainLoop::Context context)
	{
		using namespace arc::renderer;
		auto& r = context.renderer;
		bool ok;
		auto& texture_manager = r.texture_manager();

		// create shader
		id_shader = r.shader_create("../../../resources/texture_ex/texture_ex_shader.lua");
		ARC_ASSERT(id_shader != INVALID_SHADER_ID, "invalid ShaderID");

		// setup geometry config
		id_geometry_config = r.geometry_config_register(TexExampleVertexData::Layout(), IndexType::Uint16, PrimitiveType::Triangle);
		ARC_ASSERT(id_geometry_config != INVALID_GEOMETRY_CONFIG_ID, "invalid GeometryConfigID");

		// create plane geometry
		{
			UntypedBuffer buffer;
			uint16* indices;
			TexExampleVertexData* verts;

			auto id_geometry = r.geometry_create(renderer::GeometryBufferType::Static, 6, 4, id_geometry_config);
			ARC_ASSERT(id_geometry != INVALID_GEOMETRY_ID, "invalid GeometryID");

			buffer = r.geometry_map_indices(id_geometry);
			ARC_ASSERT(buffer.valid(6 * sizeof(uint16)), "index buffer mapping unsuccessful");
			indices = (uint16*)buffer.ptr;

			indices[0] = 0; indices[1] = 2; indices[2] = 1;
			indices[3] = 0; indices[4] = 2; indices[5] = 3;
			r.geometry_unmap_indices(id_geometry);

			buffer = r.geometry_map_vertices(id_geometry);
			ARC_ASSERT(buffer.valid(4 * sizeof(TexExampleVertexData)), "vertex buffer mapping unsuccessful");
			verts = (TexExampleVertexData*)buffer.ptr;

			verts[0].position = vec3{ -0.5f, -0.5f, 0.0f };
			verts[1].position = vec3{ -0.5f, 0.5f, 0.0f };
			verts[2].position = vec3{ 0.5f, 0.5f, 0.0f };
			verts[3].position = vec3{ 0.5f, -0.5f, 0.0f };

			verts[0].uv = vec2{ 0, 0 };
			verts[1].uv = vec2{ 0, 1 };
			verts[2].uv = vec2{ 1, 1 };
			verts[3].uv = vec2{ 1, 0 };

			r.geometry_unmap_vertices(id_geometry);

			id_geometry_plane = id_geometry;
		}

		// create texture
		id_tex = texture_manager.create_texture(TextureType::Texure2D, TextureFormat::fn_4_8, DIM, DIM, 0, 1);
		ARC_ASSERT(texture_manager.valid(id_tex), "invalid texture handle");

		vec4u8* tex_data = new vec4u8[DIM * DIM];
		for (uint32_t i = 0; i < DIM; i++)
		{
			for (uint32_t j = 0; j < DIM; j++)
			{
				tex_data[i * DIM + j] = vec4u8{ i, j, 0, 1 };
			}
		}

		bool success = texture_manager.set_data(id_tex, make_slice(tex_data, DIM * DIM), 0, DIM, 0, DIM);
		ARC_ASSERT(success, "texture upload unsuccessfull");
		delete[] tex_data;

		id_tex_2 = texture_manager.create_texture(TextureType::Texure2D, TextureFormat::fn_4_8, DIM2, DIM2, 0, 1);
		ARC_ASSERT(texture_manager.valid(id_tex_2), "invalid texture handle");

		tex_data = new vec4u8[DIM2 * DIM2];
		for (uint32_t i = 0; i < DIM2; i++)
		{
			for (uint32_t j = 0; j < DIM2; j++)
			{
				float a = 255.0f * (float)i / (float)DIM2;
				float b = 255.0f * (float)j / (float)DIM2;
				tex_data[i * DIM2 + j] = vec4u8{ 0, a, b, 1 };
			}
		}

		success = texture_manager.set_data(id_tex_2, make_slice(tex_data, DIM2 * DIM2), 0, DIM2, 0, DIM2);
		ARC_ASSERT(success, "texture upload unsuccessfull");
		delete[] tex_data;

		return true;
	});

	mainloop.set_update_function([&](SimpleMainLoop::Context& context, double dt)
	{
		using namespace input;
		auto& kb = context.keyboard;

		if (kb.down(Key::Escape)) return SimpleMainLoop::Status::STOP;

		return SimpleMainLoop::Status::CONTINUE;
	});

	mainloop.set_render_function([&](SimpleMainLoop::Context& context, double dt)
	{
		using namespace arc::renderer;
		auto& r = context.renderer;

		gl::clear_color(0.55f, 0.55f, 0.65f, 1.0f);
		gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
		glViewport(0, 0, context.engine_config.window_width, context.engine_config.window_height);

		// submit
		auto rb = r.render_bucket_create(25, 25 * 30 * sizeof(float));
		ARC_ASSERT(rb != nullptr, "invalid RenderBucket");

		auto color_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::vec3_t,
			SH32("instance.color"));
		ARC_ASSERT(color_offset != -1, "shader_get_uniform_offset unsuccessful");

		auto sampler_offset = r.shader_get_uniform_offset(
			id_shader,
			ShaderUniformType::Instanced,
			ShaderPrimitiveType::fsampler2D_t,
			SH32("sampler.color"));
		ARC_ASSERT(sampler_offset != -1, "shader_get_uniform_offset unsuccessful");

		for (uint32 i = 0; i < 25; i++)
		{
			auto buffer = rb->add(id_shader, id_geometry_plane, 0);
			ARC_ASSERT(buffer.valid(), "draw unsuccessful");
			buffer.access<gl44::TextureHandle>(sampler_offset) = i%2 == 0 ? id_tex : id_tex_2;
		}

		r.render_bucket_submit(rb);

		return SimpleMainLoop::Status::CONTINUE;
	});

	mainloop.run();

	std::cout << "<texture_example_end>" << std::endl;
}