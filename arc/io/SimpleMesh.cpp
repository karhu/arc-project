#include "SimpleMesh.hpp"

#include "../renderer/RendererBase.hpp"
#include "arc/logging/log.hpp"

namespace arc { namespace io {

	bool load_simple_mesh_to_gpu(renderer::RendererBase& renderer, BinaryReadStream& in, std::function<void(renderer::GeometryID)> cb)
	{
		if (!cb) return false;

		if (!in.supports_seek()) { LOG_WARNING("Input stream does not support seek"); return false; }
		if (!in.supports_tell()) { LOG_WARNING("Input stream does not support tell"); return false; }

		auto stream_begin = in.tell();

		MeshHeader header;
		PartHeader ph;
		uint32_t next_offset = 0;

		renderer::VertexAttribute attributes[16];

		attributes[0] = renderer::VertexAttribute(
			SH32("position"),
			renderer::VertexAttribute::Type::float32,
			3, next_offset);
		next_offset += 3*sizeof(float);

		in.read(&header, sizeof(MeshHeader));
		if (!valid(header)) { LOG_ERROR("Invalid SimpleMesh Header"); return false; }
		if (header.part_count == 0) { LOG_WARNING("SimpleMesh containts 0 parts."); return false; }

		for (uint32_t i = 0; i < header.part_count; i++)
		{
			in.read(&ph, sizeof(PartHeader));
			if (!valid(ph)) { LOG_ERROR("Invalid SimpleMesh Part Header"); return false; }

			uint32_t att_count = count_attributes(ph);
			if (att_count > 16) { LOG_ERROR("Mesh has too many attributes."); return false; }

			uint32_t att_idx = 1;
			if (ph.attributes.normal)
			{
				attributes[att_idx++] = renderer::VertexAttribute(
					SH32("normal"),
					renderer::VertexAttribute::Type::float32,
					3, next_offset);
				next_offset += 3 * sizeof(float);
			}
			if (ph.attributes.color1)
			{
				attributes[att_idx++] = renderer::VertexAttribute(
					SH32("color1"),
					renderer::VertexAttribute::Type::uint8,
					4, next_offset);
				next_offset += 4 * sizeof(uint8_t);
			}
			if (ph.attributes.color1)
			{
				attributes[att_idx++] = renderer::VertexAttribute(
					SH32("color2"),
					renderer::VertexAttribute::Type::uint8,
					4, next_offset);
				next_offset += 4 * sizeof(uint8_t);
			}
			if (ph.attributes.uv1)
			{
				attributes[att_idx++] = renderer::VertexAttribute(
					SH32("uv1"),
					renderer::VertexAttribute::Type::float32,
					2, next_offset);
				next_offset += 2 * sizeof(float);
			}
			if (ph.attributes.uv2)
			{
				attributes[att_idx++] = renderer::VertexAttribute(
					SH32("uv2"),
					renderer::VertexAttribute::Type::float32,
					2, next_offset);
				next_offset += 2 * sizeof(float);
			}
			// StringHash32 name_hash, VertexAttribute* attributes, uint8 count, uint8 stride
			renderer::VertexLayout vl(SH32("SimpleMeshLayout"), attributes, att_count, next_offset);
			auto it = static_cast<renderer::IndexType>(ph.index_type);
			auto gcid = renderer.geometry_config_register(vl, it, renderer::PrimitiveType::Triangle);
			auto gid = renderer.geometry_create(renderer::GeometryBufferType::Static, ph.index_count, ph.vertex_count, gcid);

			{ // index data
				auto buffer = renderer.geometry_map_indices(gid);
				uint32_t begin = index_data_begin(ph);
				uint32_t size = index_data_end(ph) - begin;
				ARC_ASSERT(buffer.size == size, "invalid index buffer size");
				in.seek_start(stream_begin + begin);
				in.read(buffer.ptr, size);
				renderer.geometry_unmap_indices(gid);
			}

			{ // vertex data
				auto buffer = renderer.geometry_map_vertices(gid);
				uint32_t begin = vertex_data_begin(ph);
				uint32_t size = vertex_data_end(ph) - begin;
				ARC_ASSERT(buffer.size == size, "invalid vertex buffer size");
				in.seek_start(stream_begin + begin);
				in.read(buffer.ptr, size);
				renderer.geometry_unmap_vertices(gid);
			}

			cb(gid);
		}
		return true;
	}
	
}}