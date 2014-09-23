#pragma once

#include "types.hpp"
#include "arc/string/StringView.hpp"

#include <array>

namespace arc { namespace meta {

	template<typename T>
	struct enum_helper
	{};

	template<>
	struct enum_helper < gl::Primitive >
	{
		using T = gl::Primitive;

		static inline T from_index(uint32 index)
		{
			T values[] = {
				T::Points, T::LineStrip, T::LineLoop, T::Lines, T::LineStripAdjacency,
				T::TriangleStrip, T::TriangleFan, T::Triangles, T::TriangleStripAdjacency, T::TrianglesAdjacency
			};

			return values[index];
		}

		static ARC_CONSTEXPR uint32 count() { return 10; }

		static ARC_CONSTEXPR StringView type_name() { return "gl::Primitive"; }

		static ARC_CONSTEXPR StringView value_name(T v)
		{
			switch (v)
			{
			case T::Points: return "Points";
			case T::LineStrip: return "LineStrip";
			case T::LineLoop: return "LineLoop";
			case T::Lines: return "Lines";
			case T::LineStripAdjacency: return "LineStripAdjacency";
			case T::TriangleStrip: return "TriangleStrip";
			case T::TriangleFan: return "TriangleFan";
			case T::Triangles: return "Triangles";
			case T::TriangleStripAdjacency: return "TriangleStripAdjacency";
			case T::TrianglesAdjacency: return "TrianglesAdjacency";
			default: return "<invalid value>";
			}
		}
		
	};

	template<>
	struct enum_helper < gl::BufferType >
	{
		using T = gl::BufferType;

		static inline T from_index(uint32 index)
		{
			T values[] = {
				T::Array, T::CopyRead, T::CopyWrite, T::ElementArray, T::PixelPack,
				T::PixelUnpack, T::Texture, T::TransformFeedback, T::Uniform, T::DrawIndirect
			};

			return values[index];
		}

		static ARC_CONSTEXPR uint32 count() { return 10; }

		static ARC_CONSTEXPR StringView type_name() { return "gl::BufferType"; }

		static ARC_CONSTEXPR StringView value_name(T v)
		{
			switch (v)
			{
			case T::Array: return "Array";
			case T::CopyRead: return "CopyRead";
			case T::CopyWrite: return "CopyWrite";
			case T::ElementArray: return "ElementArray";
			case T::PixelPack: return "PixelPack";
			case T::PixelUnpack: return "PixelUnpack";
			case T::Texture: return "Texture";
			case T::TransformFeedback: return "TransformFeedback";
			case T::Uniform: return "Uniform";
			case T::DrawIndirect: return "DrawIndirect";
			default: return "<invalid value>";
			}
		}

	};

}}