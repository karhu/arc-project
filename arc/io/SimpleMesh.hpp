#pragma once

#include <stdint.h>
#include <functional>

#include "arc/string/StringView.hpp"

#include "../renderer/types.hpp"

#include "FileStream.hpp"


namespace arc{ namespace renderer { class RendererBase; } }

namespace arc { namespace io {

	struct MeshHeader
	{
		uint32_t magic_number;
		uint32_t version;
		uint32_t part_count;

		static const uint32_t MAGIC = 42;
		static const uint32_t CURRENT_VERSION = 0;
	};

	struct PartHeader
	{
		uint64_t name_hash;
		char	 name[64]; // = { 0 };

		uint8_t index_type;
		uint8_t primitive_type;

		struct Attributes
		{
			uint32_t normal  : 1; // vec3
			uint32_t color1  : 1; // vec4u8
			uint32_t color2  : 1; // vec4u8
			uint32_t uv1     : 1; // vec2
			uint32_t uv2     : 1; // vec2
		} attributes;

		uint32_t vertex_count;
		uint32_t index_count;

		uint32_t data_offset;
	};

	inline bool valid(const PartHeader& ph)
	{
		if (ph.index_type != 4 && ph.index_type != 2 && ph.index_type != 1) return false;

		return true;
	}

	inline bool valid(const MeshHeader& mh)
	{
		return mh.magic_number == MeshHeader::MAGIC && mh.version == MeshHeader::CURRENT_VERSION;
	}

	inline uint32_t vertex_stride(const PartHeader& ph)
	{ 
		uint32 stride = 3 * sizeof(float); // position
		stride += ph.attributes.normal* 3 * sizeof(float);
		stride += ph.attributes.color1 * 4 * sizeof(uint8_t);
		stride += ph.attributes.color2 * 4 * sizeof(uint8_t);
		stride += ph.attributes.uv1 * 2 * sizeof(float);
		stride += ph.attributes.uv2 * 2 * sizeof(float);
		return stride;
	}

	inline uint32_t count_attributes(const PartHeader& ph)
	{
		auto& a = ph.attributes;
		return 1 + a.normal + a.color1 + a.color2 + a.uv1 + a.uv2;
	}

	inline uint32_t index_data_begin(const PartHeader& ph) { return ph.data_offset; }
	inline uint32_t index_data_end(const PartHeader& ph) { return ph.data_offset + ph.index_type * ph.index_count; }
	inline uint32_t index_data_size(const PartHeader& ph) { return ph.index_type * ph.index_count; }
	inline uint32_t vertex_data_begin(const PartHeader& ph) { return index_data_end(ph); }
	inline uint32_t vertex_data_end(const PartHeader& ph) { return vertex_data_begin(ph) + vertex_stride(ph)*ph.vertex_count; }
	inline uint32_t vertex_data_size(const PartHeader& ph) { return vertex_stride(ph)*ph.vertex_count; }

	/** Simple mesh loading routine.
	 * Loads a SimpleMesh from an input stream and creates a new gpu geometry for each part found. 
     * Calls the callback function for every geometry created this way. */
	bool load_simple_mesh_to_gpu(renderer::RendererBase& renderer, BinaryReadStream& in, std::function<void(renderer::GeometryID)> cb);
}}