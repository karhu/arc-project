#if 0
#pragma once

#include "arc/core.hpp"
#include "arc/lua/State.hpp"

// Type Definitions /////////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	enum class IndexType : uint8
	{
		Undefined = 0,
		Uint8 = 1,
		Uint16 = 2,
		Uint32 = 4
	};

}}

#include "GeometryManager.hpp"
struct VertexLayout2;

// Function Definitions /////////////////////////////////////////////////////////////////////////////////////////

namespace arc { namespace renderer {

	bool initialize(lua::Value& config);
	bool finalize();
	bool is_initialized();

	// vertex layout

	void invalidate_vertex_layout(VertexLayout2* ptr_layout);

	// static mesh //////
	
	template<uint8 INDEX_BITS, uint8 GENERATION_BITS>
	struct Handle32
	{
		uint32 m_index : INDEX_BITS;
		uint32 m_generation : GENERATION_BITS;

		static_assert(INDEX_BITS + GENERATION_BITS <= 32, "Total bit count exceeds 32.");
	};

	template<uint8 INDEX_BITS, uint8 GENERATION_BITS, uint8 CUSTOM_BITS>
	struct Handle32C
	{
		uint32 m_index : INDEX_BITS;
		uint32 m_generation : GENERATION_BITS;
		uint32 m_custom : CUSTOM_BITS;

		static_assert(INDEX_BITS + GENERATION_BITS + CUSTOM_BITS <= 32, "Total bit count exceeds 32.");
	};
	
	using MeshHandle = Handle32<22,10>;



	MeshHandle allocate_static_mesh(uint32 index_count, uint32 vertex_count, IndexType index_type, VertexLayout2* vertex_layout);

	void free_static_mesh(MeshHandle mesh_handle);

	/*
	struct DrawSortKey
	{
		int16 mesh_group;
		int16 mesh_index;
		int16 shader_index;
		int16 shader_config;
	};
	
	struct DrawData
	{
		uint32 entity_index;
	};

	struct DrawType
	{
		using SortkeyT = DrawSortKey;
		using DataT = DrawData;
	};


	bool initialize(lua::Value config);
	bool finalize();
	
	template<typename SortKeyT, typename DataT>
	struct DrawSubmission
	{
		bool add(Sortkey&& sortkey, DataT&& data);

	private:
		SortKeyT* m_sort_keys = nullptr;
		DataT*    m_data = nullptr;

		uint32 m_current_size = 0;
		uint32 m_max_size = 0;
	};

	template<typename SortKeyT, typename DataT>
	DrawSubmission<SortKeyT,DataT>* create_submission(uint32 n_draw_calls);

	template<typename SortKeyT, typename DataT>
	void submit_submission(DrawSubmission<SortKeyT, DataT>* submission);

	*/
}}

#endif