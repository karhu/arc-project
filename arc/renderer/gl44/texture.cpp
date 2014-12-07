#pragma once

#include "texture.hpp"
#include "arc/collections/HashMap.inl"
#include "arc/renderer/RendererConfig.hpp"


namespace arc { namespace renderer { namespace gl44 {

	TextureManager::TextureManager(const Config& config, const AllocatorConfig& allocator_config)
		: m_tex_handle_to_info(*allocator_config.longterm_allocator)
	{}

	TextureHandle TextureManager::create_texture(TextureType type, TextureFormat format, uint32_t x_dim, uint32_t y_dim, uint32_t z_dim, uint32_t mip_levels)
	{
		ARC_ASSERT(mip_levels > 0, "texture needs at least one mip level (the base level)");

		// create texture id
		uint32_t tex_id;
		gl::gen_textures(make_slice(&tex_id, 1));
		
		// allocate gpu texture memory
		switch (type)
		{
		case TextureType::Texure1D:
			gl::bind_texture(gl::TextureTarget::Texture1D, tex_id);
			gl::tex_storage_1d(gl::TextureTarget::Texture1D, mip_levels, (uint32_t)format, x_dim);
			break;
		case TextureType::Texure2D:
			gl::bind_texture(gl::TextureTarget::Texture2D, tex_id);
			gl::tex_storage_2d(gl::TextureTarget::Texture2D, mip_levels, (uint32_t)format, x_dim, y_dim);
			break;
		default:
			ARC_NOT_IMPLEMENTED;
		}

		// create texture handle
		uint64_t gl_tex_handle = gl::get_texture_handle(tex_id);
		gl::make_texture_handle_resident(gl_tex_handle);

		TextureInfo info = {
			tex_id,
			x_dim,
			y_dim,
			z_dim,
			type,
			mip_levels
		};

		// book keeping
		m_tex_handle_to_info.set(gl_tex_handle, info);
		
		TextureHandle result; result.m_value = gl_tex_handle;
		return result;
	}

	bool TextureManager::set_data(TextureHandle h, const Slice<vec4u8> data, uint32_t offset_x, uint32 count_x)
	{
		// lookup meta info
		auto entry = m_tex_handle_to_info.lookup(h.value());
		if (entry == nullptr) return false;
		auto& info = entry->value();

		// check for valid input parameters
		if (info.type != TextureType::Texure1D) return false;
		if (count_x > data.size()) return false;
		if (offset_x + count_x > info.dim_x) return false;

		// bind texture and upload data
		gl::bind_texture(gl::TextureTarget::Texture1D, info.gl_tex);
		gl::tex_sub_image_1d(gl::TextureTarget::Texture1D, 0, offset_x, count_x, GL_RGBA, GL_UNSIGNED_BYTE, data.ptr());
		
		return true;
	}

	bool TextureManager::set_data(TextureHandle h, const Slice<vec4u8> data, uint32_t offset_x, uint32 count_x, uint32_t offset_y, uint32_t count_y)
	{
		// lookup meta info
		auto entry = m_tex_handle_to_info.lookup(h.value());
		if (entry == nullptr) return false;
		auto& info = entry->value();

		// check for valid input parameters
		if (info.type != TextureType::Texure2D) return false;
		if (count_x*count_y > data.size()) return false;
		if (offset_x + count_x > info.dim_x) return false;
		if (offset_y + count_y > info.dim_y) return false;

		// bind texture and upload data
		gl::bind_texture(gl::TextureTarget::Texture2D, info.gl_tex);
		gl::tex_sub_image_2d(gl::TextureTarget::Texture2D, 0, offset_x, offset_y, count_x, count_y, GL_RGBA, GL_UNSIGNED_BYTE, data.ptr());

		return true;
	}

	bool TextureManager::valid(TextureHandle h) const
	{
		if (h.value() == 0) return false;
		auto entry = m_tex_handle_to_info.lookup(h.value());
		if (entry == nullptr) return false;
		return true;
	}

}}}