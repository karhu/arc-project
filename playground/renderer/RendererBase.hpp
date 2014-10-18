#pragma once

#include "arc/common.hpp"
#include "arc/hash/StringHash.hpp"

#include "types.hpp"
#include "VertexLayout.hpp"
#include "RendererConfig.hpp"

namespace arc { namespace renderer {

	// RenderBucketBase //////////////////////////////////////////////////////////////

	class RenderBucketBase
	{
	public:
		virtual UntypedBuffer add(ShaderID shader, GeometryID mesh, uint16 sort_depth) = 0;
	};

	// RendererBase ///////////////////////////////////////////////////////////////////

	class RendererBase
	{
	public:
		virtual bool initialize(const renderer::Config& config) = 0;
		virtual bool is_initialized() = 0;
		virtual void finalize() = 0;
	public:
		virtual StringHash64 type_hash() = 0;
	public:
		virtual void update_frame_begin() = 0;
		virtual void update_frame_end() = 0;
	public:
		virtual GeometryID geometry_create(GeometryBufferType buffer_type, uint32 index_count, uint32 vertex_count, GeometryConfigID geometry_config) = 0;
		virtual void geometry_destroy(GeometryID id) = 0;

		virtual UntypedBuffer geometry_map_vertices(GeometryID id) = 0;
		virtual UntypedBuffer geometry_map_indices(GeometryID id) = 0;
		virtual void geometry_unmap_vertices(GeometryID id) = 0;
		virtual void geometry_unmap_indices(GeometryID id) = 0;

		virtual GeometryBufferType geometry_get_buffer(GeometryID id) = 0;
		virtual GeometryConfigID geometry_get_config(GeometryID id) = 0;
		virtual uint32 geometry_get_index_count(GeometryID id) = 0;
		virtual uint32 geometry_get_vertex_count(GeometryID id) = 0;
	public:
		virtual GeometryConfigID geometry_config_register(VertexLayout* layout, IndexType index_type, PrimitiveType primitive_type) = 0;
		virtual void geometry_config_unregister(GeometryConfigID id) = 0;
		virtual VertexLayout* vertex_layout_get(GeometryConfigID id) = 0;
	public:
		virtual RenderBucketBase* render_bucket_create(uint32 max_command_count, uint32 max_data_size) = 0;
		virtual void render_bucket_submit(RenderBucketBase* render_context) = 0;
	public:
		virtual ShaderID shader_create(StringView lua_file_path) = 0;
		//virtual void shader_destroy(ShaderID id) = 0;
		virtual int32 shader_get_uniform_offset(ShaderID shader, ShaderUniformType uniform_type, ShaderPrimitiveType type, StringHash32 name) = 0;
	};

}} // namespace arc::renderer
