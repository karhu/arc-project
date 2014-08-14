// main.cpp : Defines the entry point for the console application.
//


#include <iostream>


#include "arc/core.hpp"
#include "arc/memory/Allocator.hpp"
#include "arc/logging/log.hpp"
#include "arc/string/util.hpp"

#include "engine.hpp"
#include "ShaderManager.hpp"
#include "GeometryManager.hpp"
#include "input.hpp"

#include "arc/gl/functions.hpp"
#include "arc/math/vectors.hpp"

//#include "entity/EntitySystem.hpp"


#include "entity.hpp"
#include "entity/TransformComponent.hpp"
#include "entity/SimpleMaterialComponent.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace arc;

struct CameraData
{
	vec3 origin;
	vec3 focus;
	float fov;
	float zNear;
	float zFar;
};

struct DefaultVertexData
{
	vec3   position;
	vec3   normal;
	vec4u8 color;

	static VertexLayout& Layout();
};

VertexLayout& DefaultVertexData::Layout()
{
	using Type = VertexAttribute::Type;
	static VertexAttribute attributes[] = {
		{ SH("position"),     Type::float32,  3, offsetof(DefaultVertexData, position) },
		{ SH("normal"),       Type::float32,  3, offsetof(DefaultVertexData, normal) },
		{ SH("color"),        Type::uint8_nf, 4, offsetof(DefaultVertexData, color) },
	};
	static VertexLayout layout = { "DefaultVertexData", attributes, 3, sizeof(DefaultVertexData) };
	return layout;
}

const uint32 MESH_COUNT = 2;

struct DrawCommand
{
	uint16 mesh_group;
	uint16 mesh_index;

	uint16 instance_count;

	uint16 shader;
	uint16 shader_config;
};

struct ViewData
{
	mat4 view_matrix;
	mat4 proj_matrix;
	mat4 proj_view_matrix;
	vec2 viewport;
	vec2 viewport_inv;
};

const uint32 MAX_DRAW_COMMANDS = 1024;


struct
{
	uint32 vb_id;
	uint32 vb_size;
	uint32 vao_id;
	uint32 shader_id;
	MeshData mesh_data[MESH_COUNT];

	// draw indirect buffer
	uint32 dib_id; 
	gl::DrawElementsIndirectCommand* dib_data;

	// model matrix uniform buffer
	uint32 ub_model_id;
	uint32 ub_model_size;

	// view-info uniform buffer
	uint32 ub_view_id;
	ViewData* ub_view_data;

	DrawCommand draw_commands[MAX_DRAW_COMMANDS];
	uint32 n_draw_commands;
} g_state;


engine::Config g_config;

void allocate_gpu_buffers();
bool create_vertex_array_object();
bool upload_geometry_data();
void setup_scene();
void render();
void draw();
void draw_v2();

#include "template_util.hpp"

struct TestHandler
{
	template<typename T>
	static bool iteration(uint32 index, double& user_data) { std::cout << user_data << "  " << index << ": " << sizeof(T) << std::endl; user_data += 1; return true; }
};

int main(int argc, char** argv)
{
	std::cout << "<begin>" << std::endl;

	arc::log::DefaultLogger default_logger;
	arc::log::set_logger(default_logger);

	// init engine /////////////////////////////////////////////////////////////////////
	engine::initialize(g_config);

	lua::State sys_config;

	auto& shader_manager = *engine::add_subsystem<engine::ShaderManager>();
	auto& keyboard = *engine::add_subsystem<input::KeyboardState>();

	engine::initialize_subsystems(sys_config);

	// load shader /////////////////////////////////////////////////////////////////////
	
	shader_manager.register_vertex_attribute("position",   engine::VertexInputType::Float,    3, 0);
	shader_manager.register_vertex_attribute("normal",     engine::VertexInputType::Float,    3, 1);
	shader_manager.register_vertex_attribute("color",      engine::VertexInputType::Float,    3, 2);
	shader_manager.register_vertex_attribute("data_index", engine::VertexInputType::Unsigned, 4, 10);

	auto shader = shader_manager.create_from_file("shader.lua");
	g_state.shader_id = shader.value();

	// experiment //////////////////////////////////////////////////////////////////////

	bool success;

	allocate_gpu_buffers();
	success = upload_geometry_data(); 
	ARC_ASSERT(success, "upload_geometry_data failed");
	success = create_vertex_array_object();
	ARC_ASSERT(success, "create_vertex_array_object failed");

	setup_scene();
	
	int32 counter = 60;

	bool stop = false;

	using namespace arc::input;

	while (counter > 0 && !stop)
	{
		engine::deprecated_update();
		render();

		if (keyboard.down(Key::Escape)) stop = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		--counter;
	}

	// entity test /////////////////////////////////////////////////////////////////////

	entity::initialize(sys_config);

	entity::register_component<TransformComponent>(512);
	entity::register_component<SimpleMaterialComponent>(256);

	auto eh0 = entity::create();
	auto eh1 = entity::create();
	auto eh2 = entity::create();

	auto tc0 = entity::add<TransformComponent>(eh0);
	auto tc2 = entity::add<TransformComponent>(eh2);

	ARC_ASSERT(tc0.valid(), "invalid transform component");
	ARC_ASSERT(tc2.valid(), "invalid transform component");

	tc0.set_position(vec4(1, 2, 3, 4));
	tc2.set_scale(vec4(5, 6, 7, 8));

	auto tc1 = entity::get<TransformComponent>(eh1);
	ARC_ASSERT(!tc1.valid(), "TransformComponent should be invalid");

	tc2 = entity::get<TransformComponent>(eh2);
	ARC_ASSERT(tc2.valid(), "TransformComponent should be valid");

	auto s = tc2.get_scale();
	std::cout << "scale: " << s.x << " " << s.y << " " << s.z << " " << s.w << std::endl;



	entity::finalize();

	/*

	memory::Mallocator entity_alloc;
	bool ok;
	ok = entity_system.register_component<TransformComponent>(&entity_alloc, 512);
	ok = entity_system.register_component<SimpleMaterialComponent>(&entity_alloc, 256);


	auto eh0 = entity_system.create_entity();
	auto eh1 = entity_system.create_entity();
	auto eh2 = entity_system.create_entity();

	auto tc0 = eh0.add<TransformComponent>();
	auto tc2 = eh2.add<TransformComponent>();

	tc0.set_position(vec4(1, 2, 3, 4));
	tc2.set_scale(vec4(5, 6, 7, 8));

	auto tc1 = eh1.get<TransformComponent>();
	ARC_ASSERT(!tc1.valid(), "TransformComponent should be invalid");

	tc2 = eh2.get<TransformComponent>();
	ARC_ASSERT(tc2.valid(), "TransformComponent should be valid");

	auto s = tc2.get_scale();
	std::cout << "scale: " << s.x << " " << s.y << " " << s.z << " " << s.w << std::endl;

	*/

	/////

	/*

	double tmp_double = 1.5;
	parameter_pack<uint32, uint32*, vec4, mat3>::foreach<TestHandler>(tmp_double);
	std::cout << tmp_double << std::endl;

	using TransformData = StructOfArrays <
		vec4,		// position
		vec4,		// scale
		vec4,		// orientation
		uint32		// entity index
	>;

	TransformData transform_data;
	transform_data.initialize(&entity_alloc, 512);

	*/

	// stop engine /////////////////////////////////////////////////////////////////////

	engine::shutdown();

	// close ///////////////////////////////////////////////////////////////////////////

	std::cout << "<end>" << std::endl;
	system("pause");
	return 0;
}

void setup_scene()
{
	auto& dc0 = g_state.draw_commands[0];
	dc0.instance_count = 15;
	dc0.mesh_group = g_state.vao_id;
	dc0.mesh_index = 0;
	dc0.shader = g_state.shader_id;
	dc0.shader_config = 0;

	auto& dc1 = g_state.draw_commands[1];
	dc1.instance_count = 15;
	dc1.mesh_group = g_state.vao_id;
	dc1.mesh_index = 1;
	dc1.shader = g_state.shader_id;
	dc1.shader_config = 0;

	g_state.n_draw_commands = 2;
}

void render()
{
	gl::clear_color(0.35f, 0.35f, 0.35f, 1.0f);
	gl::clear(gl::ClearBufferBits::Color | gl::ClearBufferBits::Depth);
	glViewport(0, 0, g_config.window_width, g_config.window_height);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	draw_v2();

	engine::deprecated_swap();
}

void allocate_gpu_buffers()
{
	uint32 vertex_size = 1024 * 1024 * 24; // 24MB
	uint32 index_size = 1024 * 1024 * 8; // 8MB
	g_state.vb_size = vertex_size + index_size;

	auto target = gl::BufferType::Array;
	gl::create_buffers(1, &g_state.vb_id);
	gl::bind_buffer(target, g_state.vb_id);
	gl::buffer_data(target, g_state.vb_size, gl::BufferUsage::StaticDraw);


	// Draw Indirect Buffer ///////////////////////////////////////////////////

	target = gl::BufferType::DrawIndirect;
	gl::create_buffers(1, &g_state.dib_id);
	gl::bind_buffer(target, g_state.dib_id);

	uint32 draw_commands_size = MAX_DRAW_COMMANDS*sizeof(gl::DrawElementsIndirectCommand);

	gl::buffer_storage(target, draw_commands_size, 0,
		gl::BufferStorage::Write |
		gl::BufferStorage::Persistent |
		gl::BufferStorage::Coherent);

	void* ptr = gl::map_buffer_range(target, 0, draw_commands_size,
					gl::BufferAccess::Write |
					gl::BufferAccess::Persistent |
					gl::BufferAccess::Coherent);

	g_state.dib_data = (gl::DrawElementsIndirectCommand*)ptr;

	// model matrix uniform buffer /////////////////////////////////////////////

	target = gl::BufferType::Uniform;
	gl::create_buffers(1, &g_state.ub_model_id);
	gl::bind_buffer(target, g_state.ub_model_id);

	uint32 mm_buffer_size = sizeof(float) * 16 * 1024 * 16;
	g_state.ub_model_size = mm_buffer_size;

	gl::buffer_storage(target, mm_buffer_size, 0,
		gl::BufferStorage::Write);

	/*
	void* ptr = gl::map_buffer_range(target, 0, mm_buffer_size,
		gl::BufferAccess::Write);

	g_state.dib_data = (gl::DrawElementsIndirectCommand*)ptr;

	*/

	// camera info uniform buffer //////////////////////////////////////////////

	target = gl::BufferType::Uniform;
	gl::create_buffers(1, &g_state.ub_view_id);
	gl::bind_buffer(target, g_state.ub_view_id);

	gl::buffer_storage(target, 3*sizeof(ViewData), 0,
		gl::BufferStorage::Write |
		gl::BufferStorage::Persistent |
		gl::BufferStorage::Coherent);

	ptr = gl::map_buffer_range(target, 0, 3*sizeof(ViewData),
		gl::BufferAccess::Write |
		gl::BufferAccess::Persistent |
		gl::BufferAccess::Coherent);

	g_state.ub_view_data = (ViewData*)ptr;

}

uint32 forward_to_alignment(uint32 begin, uint32 alignment)
{
	uint32 mod = begin % alignment;
	if (mod == 0) return begin;
	else return begin + alignment - mod;
}

uint32 index_stride(IndexType type)
{
	switch (type)
	{
	case IndexType::Uint8: return 1;
	case IndexType::Uint16: return 2;
	case IndexType::Uint32: return 4;
	}
	ARC_ASSERT(false, "Invalid IndexType");
	return 4;
}

bool upload_geometry_data()
{
	auto target = gl::BufferType::Array;
	gl::bind_buffer(target, g_state.vb_id);

	auto& mesh_data = g_state.mesh_data;

	// square
	mesh_data[0].index_count = 6;
	mesh_data[0].vertex_count = 4;
	mesh_data[0].index_type = IndexType::Uint16;
	mesh_data[0].vertex_layout = &DefaultVertexData::Layout();

	// diamond
	mesh_data[1].index_count = 12;
	mesh_data[1].vertex_count = 5;
	mesh_data[1].index_type = IndexType::Uint16;
	mesh_data[1].vertex_layout = &DefaultVertexData::Layout();

	uint32 next_free = 0;

	void* ptr;
	DefaultVertexData* vd_ptr;
	uint16* id_ptr;
	uint32 begin, end;

	// square
	{
		auto& mesh = mesh_data[0];
		auto stride = mesh.vertex_layout->stride();
		auto idx_stride = index_stride(mesh.index_type);

		end = next_free;

		// index data
		begin = forward_to_alignment(end, idx_stride);
		end = begin + mesh.index_count * idx_stride;

		mesh.index_offset = begin / idx_stride;

		if (end >= g_state.vb_size) return false;

		ptr = gl::map_buffer_range(target, begin, end - begin,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);
		if (ptr == nullptr) return false;
		if (mesh.index_type != IndexType::Uint16) return false;
		
		id_ptr = (uint16*)ptr;
		id_ptr[0] = 0; id_ptr[1] = 1; id_ptr[2] = 2;
		id_ptr[3] = 0; id_ptr[4] = 2; id_ptr[5] = 3;
		
		gl::unmap_buffer(target);

		// vertex data
		begin = forward_to_alignment(end, stride);
		end = begin + mesh.vertex_count*stride;

		mesh.vertex_offset = begin / stride;

		if (end >= g_state.vb_size) return false;

		ptr = gl::map_buffer_range(target, begin, end - begin,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);
		if (ptr == nullptr) return false;

		vd_ptr = (DefaultVertexData*)ptr;
		vd_ptr[0].position = vec3{ -0.5f, -0.5f, 0.0f }; vd_ptr[0].color = vec4u8{ 255, 0, 0, 255 };
		vd_ptr[1].position = vec3{ -0.5f, 0.5f, 0.0f }; vd_ptr[1].color = vec4u8{ 0, 255, 0, 255 };
		vd_ptr[2].position = vec3{ 0.5f, 0.5f, 0.0f }; vd_ptr[2].color = vec4u8{ 0, 0, 255, 255 };
		vd_ptr[3].position = vec3{ 0.5f, -0.5f, 0.0f }; vd_ptr[3].color = vec4u8{ 255, 255, 0, 255 };

		gl::unmap_buffer(target);
	}

	// diamond
	{
		auto& mesh = mesh_data[1];
		auto stride = mesh.vertex_layout->stride();
		auto idx_stride = index_stride(mesh.index_type);

		// index data
		begin = forward_to_alignment(end, idx_stride);
		end = begin + mesh.index_count * idx_stride;

		mesh.index_offset = begin / idx_stride;

		if (end >= g_state.vb_size) return false;

		ptr = gl::map_buffer_range(target, begin, end - begin,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);
		if (ptr == nullptr) return false;
		if (mesh.index_type != IndexType::Uint16) return false;

		id_ptr = (uint16*)ptr;
		id_ptr[0] = 0; id_ptr[1] = 1;  id_ptr[2] = 2;
		id_ptr[3] = 0; id_ptr[4] = 2;  id_ptr[5] = 3;
		id_ptr[6] = 0; id_ptr[7] = 3;  id_ptr[8] = 4;
		id_ptr[9] = 0; id_ptr[10] = 4; id_ptr[11] = 1;

		gl::unmap_buffer(target);

		// vertex data
		begin = forward_to_alignment(end, stride);
		end = begin + mesh.vertex_count*stride;

		mesh.vertex_offset = begin / stride;

		if (end >= g_state.vb_size) return false;

		ptr = gl::map_buffer_range(target, begin, end - begin,
			gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);
		if (ptr == nullptr) return false;

		vd_ptr = (DefaultVertexData*)ptr;
		vd_ptr[0].position = vec3{ 0.0f, 0.0f, 0.0f }; vd_ptr[0].color = vec4u8{ 255, 255, 255, 255 };
		vd_ptr[1].position = vec3{-0.5f, 0.0f, 0.0f }; vd_ptr[1].color = vec4u8{ 255,   0,   0, 255 };
		vd_ptr[2].position = vec3{ 0.0f, 0.4f, 0.0f }; vd_ptr[2].color = vec4u8{   0, 255,   0, 255 };
		vd_ptr[3].position = vec3{ 0.5f, 0.0f, 0.0f }; vd_ptr[3].color = vec4u8{   0,   0, 255, 255 };
		vd_ptr[4].position = vec3{ 0.0f,-0.4f, 0.0f }; vd_ptr[4].color = vec4u8{ 255, 255,   0, 255 };

		gl::unmap_buffer(target);
	}
	
	return true;
}

bool create_vertex_array_object()
{
	auto& layout = DefaultVertexData::Layout();

	auto sm_ptr = engine::get_subsystem<engine::ShaderManager>();
	if (!sm_ptr) return false;
	auto& sm = *sm_ptr;

	gl::create_vertex_arrays(1, &g_state.vao_id);
	gl::bind_vertex_array(g_state.vao_id);

	gl::bind_buffer(gl::BufferType::Array, g_state.vb_id);
	gl::bind_buffer(gl::BufferType::ElementArray, g_state.vb_id);

	// TODO: more verification about compatibility between vertex data and shader input format
	// TODO: check that there is no attribute location overlap

	for (uint32 i = 0; i < layout.count(); i++)
	{
		auto& att = layout.attribute(i);
		auto si_ptr = sm.get_vertex_attribute(att.hash());
		if (si_ptr == nullptr) return false;
		auto& si = *si_ptr;

		gl::enable_vertex_attribute(si.location);

		// floating point shader input
		if (si.type == engine::VertexInputType::Float)
		{
			gl::vertex_attrib_pointer(si.location,
				att.elements(),
				type_gl(att.type()),
				type_normalize(att.type()),
				layout.stride(),
				att.offset()
			);
		}
		// integer shader input
		else
		{
			gl::vertex_attrib_i_pointer(si.location,
				att.elements(),
				type_gl(att.type()),
				layout.stride(),
				att.offset()
			);
		}
	}

	gl::bind_vertex_array(0);

	return true;
}

void draw()
{
	if (g_state.n_draw_commands == 0) return;

	// buffer attributes
	uint32 primitive_type = GL_TRIANGLES;
	auto primitive_t = gl::Primitive::Triangles;

	uint32 instance_offset = 0;
	uint32 last_mg = -1;
	uint32 last_sh = -1;

	for (uint32 i = 0; i < g_state.n_draw_commands; i++)
	{
		auto& dc = g_state.draw_commands[i];
		auto& md = g_state.mesh_data[dc.mesh_index];

		if (last_mg != dc.mesh_group)
		{
			gl::bind_vertex_array(dc.mesh_group);
			last_mg = g_state.vao_id;
		}
		if (last_sh != dc.shader)
		{
			gl::use_program(dc.shader);
			last_sh = dc.shader;
		}

		gl::draw_elements_instanced_bv_bi(
			primitive_t,
			md.index_count,
			type_gl(md.index_type),
			md.index_offset*index_stride(md.index_type),
			dc.instance_count,
			md.vertex_offset,
			instance_offset);

		instance_offset += dc.instance_count;
	}

	
	gl::bind_vertex_array(0);
}

void draw_v2()
{
	// TODO sort and split by:
	// shader, buffer, data_config(vao, idx_type, primitive_type)

	// model matrix uniform buffer setup //

	auto target = gl::BufferType::Uniform;
	uint32 mm_buffer_size = sizeof(float) * 16 * 256;

	gl::bind_buffer(target, g_state.ub_model_id);

	void* ptr = gl::map_buffer_range(target, 0, mm_buffer_size,
		gl::BufferAccess::Write | gl::BufferAccess::InvalidateRange);
	ARC_ASSERT(ptr, "unable to map buffer");
	mat4* mm_data = (mat4*)ptr;

	for (uint32 i = 0; i < 256; i++)
	{
		float v = 0.8f;

		mm_data[i] = mat4(v,0,0,0,
					      0,v,0,0,
					      0,0,v,0,
					      0,0,0,1);
	}
	
	gl::unmap_buffer(target);

	gl::bind_buffer_range(gl::ShaderBufferTarget::Uniform,
		3, g_state.ub_model_id, 0, sizeof(float) * 16 * 256);

	// uniform view data setup //

	auto& vd = g_state.ub_view_data[0];

	vd.viewport = vec2(1280, 720);
	vd.viewport_inv = vec2(1.0 / 1280, 1.0 / 720);

	float v = 0.8f;
	vd.view_matrix = glm::lookAt(vec3(0, 1, -1), vec3(0, 0, 0), vec3(0, 1, 0));
	vd.proj_matrix = glm::perspective(58.0f, (float)g_config.window_width / (float)g_config.window_height, 0.25f, 1000.0f);
	vd.proj_view_matrix = vd.proj_matrix * vd.view_matrix;
	

	gl::bind_buffer_range(gl::ShaderBufferTarget::Uniform,
		4, g_state.ub_view_id, 0, sizeof(ViewData));

	// draw command setup //

	uint32 instance_offset = 0;

	for (uint32 i = 0; i < g_state.n_draw_commands; i++)
	{
		uint32 idx = 1 - i;
		auto& dc = g_state.draw_commands[i];
		auto& md = g_state.mesh_data[dc.mesh_index];

		auto& cmd = g_state.dib_data[i];

		cmd.count = md.index_count;
		cmd.first_index = md.index_offset;
		cmd.base_vertex = md.vertex_offset;

		cmd.instance_count = dc.instance_count;
		cmd.base_instance = instance_offset;

		instance_offset += dc.instance_count;
	}

	gl::bind_buffer(gl::BufferType::DrawIndirect, g_state.dib_id);
	gl::bind_vertex_array(g_state.vao_id);
	gl::use_program(g_state.shader_id);

	gl::multi_draw_elements_indirect(gl::Primitive::Triangles, gl::IndexType::u16, 0,
		g_state.n_draw_commands, sizeof(gl::DrawElementsIndirectCommand));
}

 