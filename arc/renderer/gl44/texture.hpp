#pragma once

#include "arc/common.hpp"
#include "arc/collections/HashMap.hpp"
#include "arc/gl/types.hpp"
#include "arc/gl/functions.hpp"

#include "arc/math/vectors.hpp"

#include "texture_enums.hpp"

namespace arc { namespace renderer { 
	
	struct Config;
	struct AllocatorConfig;
	
namespace gl44 {

	class TextureManager;

	struct TextureHandle
	{
		inline uint64_t value() { return m_value; }
	private:
		uint64_t m_value = 0;
		friend class TextureManager;
	};

	class TextureManager
	{
	public:
		TextureManager(const Config& config, const AllocatorConfig& allocator_config);
	public:
		/** \brief Create a new texture with specified format and dimension.
		 ** \param[in]   type     Currently only distinguishes between 1D and 2D textures. More types might follow in the future
		 ** \param[in]   format   Specifies the internal storage format as well as the type within the shader. For more info see \c TextureFormat.
		 ** \returns              A texture handle that should be checked for validity using the TextureBackend::valid(...) function. The returned 
		 **                       TextureHandle can be directly passed as a shader uniform. */
		TextureHandle create_texture(TextureType type, TextureFormat format, uint32_t x_dim, uint32_t y_dim, uint32_t z_dim, uint32_t mip_levels = 1);
	public:
		/** \brief Upload data to 1D texture memory.
		 ** \param[in]   h          Handle to the texture that should be modified.
		 ** \param[in]   data       Slice of memory that should be read from. Needs to be at least count_x entries large.
		 ** \param[in]   offset_x   Offset in pixels within the texture that should be written to.
		 ** \param[in]   count_x    Number of pixels that are written, starting at offset_x.
		 ** \returns                True on success. */
		bool set_data(TextureHandle h, const Slice<vec4u8> data, uint32_t offset_x, uint32 count_x);

		/** \brief Upload data to 2D texture memory.
		 ** \param[in]   h          Handle to the texture that should be modified.
		 ** \param[in]   data       Slice of memory that should be read from. Needs to be at least count_x*count_y entries large.
		 ** \param[in]   offset_x   Pixel offset in the x-dimension within the texture that should be written to.
		 ** \param[in]   count_x    Number of pixels that are written along the x-dimension, starting at offset_x.
		 ** \param[in]   offset_y   Pixel offset in the y-dimension within the texture that should be written to.
		 ** \param[in]   count_y    Number of pixels that are written along the y-dimension, starting at offset_y.
		 ** \returns                True on success. */
		bool set_data(TextureHandle h, const Slice<vec4u8> data, uint32_t offset_x, uint32 count_x, uint32_t offset_y, uint32_t count_y);
	public:
		bool valid(TextureHandle h) const;
	private:
		struct TextureInfo
		{
			uint32_t gl_tex;
			uint32_t dim_x;
			uint32_t dim_y;
			uint32_t dim_z;
			TextureType type;
			uint32_t mip_levels;
		};

		arc::HashMap<TextureInfo> m_tex_handle_to_info;
	};

}

}}
