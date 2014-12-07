#pragma once

#include "arc/gl/types.hpp"

namespace arc { namespace renderer {

	enum class TextureType : uint32_t
	{
		Texure1D = GL_TEXTURE_1D,
		Texure2D = GL_TEXTURE_2D,
	};

	enum class TextureFormat : uint32_t
	{
		// floating point components, backed by floats
		f_1_16 = GL_R16F,
		f_2_16 = GL_RG16F,
		f_3_16 = GL_RGB16F,
		f_4_16 = GL_RGBA16F,
		f_1_32 = GL_R32F,
		f_2_32 = GL_RG32F,
		f_3_32 = GL_RGB32F,
		f_4_32 = GL_RGBA32F,

		// normalized [0,1] floats, backed by unsigned integers
		fn_1_8 = GL_R8,
		fn_2_8 = GL_RG8,
		fn_3_8 = GL_RGB8,
		fn_4_8 = GL_RGBA8,
		fn_1_16 = GL_R16,
		fn_2_16 = GL_RG16,
		fn_3_16 = GL_RGB16,
		fn_4_16 = GL_RGBA16,

		// signed normalized [-1,1] floats, backed by signed integers
		fsn_1_8 = GL_R8_SNORM,
		fsn_2_8 = GL_RG8_SNORM,
		fsn_3_8 = GL_RGB8_SNORM,
		fsn_4_8 = GL_RGBA8_SNORM,
		fsn_1_16 = GL_R16_SNORM,
		fsn_2_16 = GL_RG16_SNORM,
		fsn_3_16 = GL_RGB16_SNORM,
		fsn_4_16 = GL_RGBA16_SNORM,

		// unsigned integer components, backed by unsigned integers
		ui_1_8 = GL_R8UI,
		ui_2_8 = GL_RG8UI,
		ui_3_8 = GL_RGB8UI,
		ui_4_8 = GL_RGBA8UI,
		ui_1_16 = GL_R16UI,
		ui_2_16 = GL_RG16UI,
		ui_3_16 = GL_RGB16UI,
		ui_4_16 = GL_RGBA16UI,
		ui_1_32 = GL_R32UI,
		ui_2_32 = GL_RG32UI,
		ui_3_32 = GL_RGB32UI,
		ui_4_32 = GL_RGBA32UI,

		// signed integer components, backed by signed integers
		i_1_8 = GL_R8I,
		i_2_8 = GL_RG8I,
		i_3_8 = GL_RGB8I,
		i_4_8 = GL_RGBA8I,
		i_1_16 = GL_R16I,
		i_2_16 = GL_RG16I,
		i_3_16 = GL_RGB16I,
		i_4_16 = GL_RGBA16I,
		i_1_32 = GL_R32I,
		i_2_32 = GL_RG32I,
		i_3_32 = GL_RGB32I,
		i_4_32 = GL_RGBA32I,

		// the odd ones
		fn_3_4 = GL_RGB4,
		fn_3_5 = GL_RGB5,
		fn_3_10 = GL_RGB10,
		fn_3_12 = GL_RGB12,
		fn_3_x332 = GL_R3_G3_B2,

		fn_4_2 = GL_RGBA2,
		fn_4_4 = GL_RGBA4,
		fn_4_12 = GL_RGBA12,

		fn_4_x5551 = GL_RGB5_A1,
		fn_4_x1010102 = GL_RGB10_A2,

		srgb_3_8 = GL_SRGB8,
		srgb_4_8 = GL_SRGB8_ALPHA8,

		fn_3_9e5 = GL_RGB9_E5, // 9bits each + 5bits shared exponent?

		ui_4_x1010102 = GL_RGB10_A2UI,
		f_3_x111110 = GL_R11F_G11F_B10F,

	};


}}