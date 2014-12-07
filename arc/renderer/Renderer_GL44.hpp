#pragma once

#include "arc/collections/Array.hpp"
#include "arc/collections/HashMap.hpp"
#include "arc/memory/LinearAllocator.hpp"
#include "arc/util/Counter.hpp"

#include "RendererBase.hpp"

#include "gl44/shader.hpp"
#include "gl44/texture.hpp"

namespace arc { namespace renderer {

	class Renderer_GL44;
	class RenderBucket_GL44;
	struct RenderCommand_GL44;

	// RenderBucket ////////////////////////////////////////////////////////////////

	struct RenderCommand_GL44
	{
		uint64 sort_key;
		void* data;
	};

	class RenderBucket_GL44 final : public RenderBucketBase
	{
	public:
		UntypedBuffer  add(ShaderID shader, GeometryID mesh, uint16 sort_depth) override;
	private:
		char* m_data_begin;
		char* m_data_end;
		char* m_data_current;

		RenderCommand_GL44* m_commands;
		uint32 m_max_command_count;
		uint32 m_count;
		Renderer_GL44* m_renderer;
	private:
		static const uint32 DRAW_DATA_ALIGNMENT = 8;
	private:
		friend class Renderer_GL44;
	};

	// Renderer ///////////////////////////////////////////////////////////////////

	class Renderer_GL44 final : public RendererBase
	{
	public:
		static bool Validate(const Config& config);
	public:
		Renderer_GL44(const Config& config, const AllocatorConfig& allocator_config);
		~Renderer_GL44() override;
	private:
		bool initialize(const Config& config, const AllocatorConfig& allocator_config);
		void finalize();
	public:
		StringHash64 type_hash() override;
	public:
		void update_frame_begin() override;
		void update_frame_end() override;
	public:
		GeometryID geometry_create(GeometryBufferType buffer_type, uint32 index_count, uint32 vertex_count, GeometryConfigID geometry_config) override;
		void geometry_destroy(GeometryID id) override;
		
		UntypedBuffer geometry_map_vertices(GeometryID id) override;
		UntypedBuffer geometry_map_indices(GeometryID id) override;
		void geometry_unmap_vertices(GeometryID id) override;
		void geometry_unmap_indices(GeometryID id) override;

		GeometryBufferType geometry_get_buffer(GeometryID id) override;
		GeometryConfigID geometry_get_config(GeometryID id) override;
		uint32 geometry_get_index_count(GeometryID id) override;
		uint32 geometry_get_vertex_count(GeometryID id) override;
	public:
		GeometryConfigID geometry_config_register(const VertexLayout& layout, IndexType index_type, PrimitiveType primitive_type) override;
		void geometry_config_unregister(GeometryConfigID id) override;
		VertexLayout* vertex_layout_get(GeometryConfigID id) override;
	public:
		RenderBucket_GL44* render_bucket_create(uint32 max_command_count, uint32 max_data_size) override;
		void render_bucket_submit(RenderBucketBase* bucket) override;
	public:
		ShaderID shader_create(StringView lua_file_path) override;
		//void shader_destroy(ShaderID id) override;
		int32 shader_get_uniform_offset(ShaderID shader, ShaderUniformType uniform_type, ShaderPrimitiveType type, StringHash32 name) override;
	public:
		inline gl44::TextureManager& texture_manager() { return m_texture_backend; }
	private:
		memory::Allocator* m_alloc;
		memory::LinearAllocator m_frame_alloc;
	private:
		gl44::TextureManager m_texture_backend;
	private:
		bool m_is_initialized = false;
	private:
		struct GeometryBufferData
		{
			uint32 size;
			uint32 gl_id;
			uint32 front;
		};
		GeometryBufferData m_geom_buffer_data[(uint32)GeometryBufferType::COUNT];
		uint32 m_geom_buffer_gl_ids[(uint32)GeometryBufferType::COUNT];
	private:
		struct GeometryData
		{
			uint32		vertex_begin;
			uint32      vertex_begin_count;
			uint32		vertex_count;
			uint32		vertex_size;

			uint32		index_begin;
			uint32      index_begin_count;
			uint32		index_count;
			uint32		index_size;

			uint32 block_begin;
			uint32 block_size;

			GeometryConfigID geometry_config_id;

			GeometryBufferType buffer_type;
			uint32 gl_id;
		};
		IndexPool32 m_geometry_indices;
		Array<GeometryData> m_geometry_data;
	private:
		struct GeometryConfig
		{
			VertexLayout*  layout;
			PrimitiveType  primitive;
			IndexType	   index_type;
			uint32		   gl_vao;

			inline bool operator==(GeometryConfig other) const { return layout == other.layout && primitive == other.primitive && index_type == other.index_type; }
		};
		CompactPool<GeometryConfig> m_geometry_config_data;
		Array<VertexLayout*> m_vertex_layouts;
	private:
		struct VertexLayoutData
		{
			VertexLayout* ref = nullptr;
		};
		uint32 m_vao_gl_ids[256];
	private:
		struct DefaultCommandData
		{
			uint32 size;
			GeometryID geometry;
		};
		Array<RenderBucket_GL44*> m_submitted_render_buckets;
		friend class RenderBucket_GL44;
	private:
		Counter32 m_frame_counter;
	private:
		gl44::ShaderBackend m_shader_backend;
	private:
		void shader_grab_instance_data(uint32 material_id, void* cmd_data);
	private:
		struct RenderState
		{
			ShaderID shader = INVALID_SHADER_ID;
			GeometryBufferType buffer = GeometryBufferType::COUNT;
			IndexType index_type = IndexType::None;
			PrimitiveType primitive_type = PrimitiveType::Unknown;
			VertexLayout* vertex_layout = nullptr;
		};
		uint32 render_state_switch(RenderState& current, RenderCommand_GL44* commands, uint32 max_count);

		uint32 m_gl_dib;
		UntypedBuffer m_gl_dib_data;
	};

}} // namespace arc::renderer
